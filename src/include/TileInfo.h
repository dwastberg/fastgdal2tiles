//
// Created by Dag Wästberg on 2024-03-06.
//

#ifndef FASTGDAL2TILES_TILEINFO_H
#define FASTGDAL2TILES_TILEINFO_H

#include "GlobalMercator.h"
#include <filesystem>
#include <stdlib.h>
#include <tuple>
#include <unordered_map>

#include <gdal_priv.h>

#define MAX_ZOOM 32

namespace FASTTILER {
    struct tile_details {
        size_t querysize;
        size_t rx;
        size_t ry;
        size_t rxsize;
        size_t rysize;

        size_t tx;
        size_t ty;
        size_t tz;

        size_t wx;
        size_t wy;
        size_t wxsize;
        size_t wysize;
    };
    typedef std::tuple<size_t, size_t, size_t> tile_id_t;

    struct TileIDHash {
        std::size_t operator()(const tile_id_t &t) const {
            std::hash<size_t> hasher;
            std::size_t seed = 0;
            seed ^= hasher(std::get<0>(t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(std::get<1>(t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(std::get<2>(t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };


    typedef std::unordered_map<tile_id_t, FASTTILER::tile_details, TileIDHash> td_map_t;
    typedef std::vector<FASTTILER::tile_details> td_vec_t;
    typedef std::pair<size_t, size_t> tile_pos_t;
    typedef std::pair<tile_id_t, tile_pos_t> tile_pos_pair_t;
    typedef std::vector<tile_pos_pair_t> overview_tile_parts_t;
    typedef std::unordered_map<size_t, std::unordered_map<tile_id_t, overview_tile_parts_t, TileIDHash>> tile_pyramid_t;
    typedef std::vector<std::tuple<size_t, size_t, size_t, size_t>> tzminmax_t;
    typedef std::pair<std::tuple<size_t, size_t, size_t, size_t>, std::tuple<size_t, size_t, size_t, size_t>> geo_query_t;


    class TileInfo {
    public:
        td_vec_t td_vec;
        td_map_t td_map;
        tile_pyramid_t tile_pyramid;
        int min_zoom;
        int max_zoom;
        size_t tile_size;
        std::string in_raster;
        tzminmax_t tzminmax;
        double ominx, omaxy, omaxx, ominy;
        double pixel_size;
        size_t raster_xsize;
        size_t raster_ysize;
        double north, south, east, west;
        double transform[6];

        TileInfo(const std::string in_raster, int min_zoom, int max_zoom, size_t tile_size = 256) {
            GDALAllRegister();
            this->in_raster = in_raster;
            this->gm = GlobalMercator();
            this->tile_size = tile_size;

            this->raster_ds = (GDALDataset *) GDALOpen(in_raster.c_str(), GA_ReadOnly);
            if (this->raster_ds == nullptr) {
                throw std::runtime_error("Could not open file");
            }
            this->get_raster_bounds();
            if (min_zoom >= 0) {
                this->min_zoom = min_zoom;
                this->max_zoom = max_zoom;
            } else {
                double min_pixel_size = this->pixel_size * (std::max(this->raster_xsize, this->raster_ysize) / this->tile_size);
                this->min_zoom = gm.ZoomForPixelSize(min_pixel_size);
                this->max_zoom = gm.ZoomForPixelSize(this->pixel_size);
            }
            GDALClose(raster_ds);
            build_tzminmax();
            build_tile_details();
            build_tile_pyramid();
        }

        tzminmax_t build_tzminmax() {
            tzminmax.clear();
            size_t tminx, tminy, tmaxx, tmaxy;
            for (size_t tz = 0; tz < MAX_ZOOM; tz++) {
                std::tie(tminx, tminy) = gm.MetersToTile(ominx, ominy, tz);
                std::tie(tmaxx, tmaxy) = gm.MetersToTile(omaxx, omaxy, tz);
                tminx = std::max(tminx, 0ul);
                tminy = std::max(tminy, 0ul);
                size_t tz_max = static_cast<size_t>(std::pow(2.0, tz) - 1);

                tmaxx = std::min(tz_max, tmaxx);
                tmaxy = std::min(tz_max, tmaxy);
                tzminmax.push_back(std::make_tuple(tminx, tminy, tmaxx, tmaxy));
            }

            return tzminmax;
        }

        td_map_t build_tile_details() {


            td_map.clear();
            auto tz = this->max_zoom;
            size_t tminx, tminy, tmaxx, tmaxy;
            size_t ti = 0;

            build_tzminmax();
            size_t ct = 0;
            std::tie(tminx, tminy, tmaxx, tmaxy) = tzminmax[tz];
            size_t tcount = (1 + (tmaxx - tminx)) * (1 + (tmaxy - tminy));
            size_t x_tiles = (tmaxx - tminx) + 1;
            size_t y_tiles = (tmaxy - tminy) + 1;
            for (int ty = tmaxy; ty >= tminy; --ty) {
                for (int tx = tminx; tx <= tmaxx; ++tx) {
                    auto b = gm.TileBounds(tx, ty, tz);
                    auto gq = geo_query(std::get<0>(b), std::get<3>(b), std::get<2>(b), std::get<1>(b), this->tile_size);
                    auto rb = gq.first;
                    auto wb = gq.second;
                    auto td = tile_details{this->tile_size, std::get<0>(rb), std::get<1>(rb), std::get<2>(rb), std::get<3>(rb), static_cast<size_t>(tx), static_cast<size_t>(ty), static_cast<size_t>(tz), std::get<0>(wb), std::get<1>(wb), std::get<2>(wb), std::get<3>(wb)};
                    td_vec.push_back(td);
                    tile_id_t tile_id = std::make_tuple(tx, ty, tz);
                    td_map[tile_id] = td;
                    ct++;
                    std::cout << "Tile " << ct << " of " << tcount << " done" << std::endl;
                }
            }
            std::cout << "Tile count: " << td_map.size() << std::endl;
            return td_map;
        }


    private:
        GDALDataset *raster_ds;
        GlobalMercator gm;

        geo_query_t geo_query(double ulx, double uly, double lrx, double lry, size_t querysize) {
            size_t wxsize, wysize;
            int rx = static_cast<size_t>((ulx - transform[0]) / transform[1] + 1e-4);
            int ry = static_cast<size_t>((uly - transform[3]) / transform[5] + 1e-4);
            int rxsize = static_cast<size_t>((lrx - ulx) / transform[1] + 0.5);
            int rysize = static_cast<size_t>((lry - uly) / transform[5] + 0.5);

            if (querysize == 0) {
                wxsize = rxsize;
                wysize = rysize;
            } else {
                wxsize = querysize;
                wysize = querysize;
            }
            size_t wx = 0;
            if (rx < 0) {
                size_t rxshift = abs(rx);
                wx = static_cast<size_t>(wxsize * (static_cast<double>(rxshift) / rxsize));
                wxsize -= wx;
                rxsize = rxsize - static_cast<size_t>(rxsize * (static_cast<double>(rxshift) / rxsize));
                rx = 0;
            }
            if (rx + rxsize > raster_xsize) {
                wxsize = static_cast<size_t>(wxsize * (static_cast<double>(raster_xsize - rx) / rxsize));
                rxsize = raster_xsize - rx;
            }

            size_t wy = 0;
            if (ry < 0) {
                size_t ryshift = abs(ry);
                wy = static_cast<size_t>(wysize * (static_cast<double>(ryshift) / rysize));
                wysize -= wy;
                rysize = rysize - static_cast<size_t>(rysize * (static_cast<double>(ryshift) / rysize));
                ry = 0;
            }
            if (ry + rysize > raster_ysize) {
                wysize = static_cast<size_t>(wysize * (static_cast<double>(raster_ysize - ry) / rysize));
                rysize = raster_ysize - ry;
            }

            auto r_query = std::make_tuple(rx, ry, rxsize, rysize);
            auto w_query = std::make_tuple(wx, wy, wxsize, wysize);
            return std::make_pair(r_query, w_query);
        }

        void get_raster_bounds() {
            raster_ds->GetGeoTransform(this->transform);

            this->ominx = transform[0];
            this->omaxx = transform[0] + raster_ds->GetRasterXSize() * transform[1];
            this->ominy = transform[3] + raster_ds->GetRasterYSize() * transform[5];
            this->omaxy = transform[3];

            this->pixel_size = transform[1];
            this->raster_xsize = raster_ds->GetRasterXSize();
            this->raster_ysize = raster_ds->GetRasterYSize();

            std::tie(this->south, this->west) = gm.MetersToLatLon(ominx, ominy);
            std::tie(this->north, this->east) = gm.MetersToLatLon(omaxx, omaxy);
        }
        void build_tile_pyramid() {
            constexpr size_t tile_size = 256;
            for (size_t tz = max_zoom - 1; tz >= min_zoom; tz--) {
                size_t tminx = std::get<0>(tzminmax[tz]);
                size_t tminy = std::get<1>(tzminmax[tz]);
                size_t tmaxx = std::get<2>(tzminmax[tz]);
                size_t tmaxy = std::get<3>(tzminmax[tz]);
                for (size_t ty = tmaxy; ty >= tminy; ty--) {
                    for (size_t tx = tminx; tx <= tmaxx; tx++) {
                        auto low_tile_id = std::make_tuple(tx, ty, tz);
                        for (size_t y_off = 0; y_off < 2; y_off++) {
                            for (size_t x_off = 0; x_off < 2; x_off++) {
                                auto y = ty * 2 + y_off;
                                auto x = tx * 2 + x_off;
                                tile_id_t tile_id = std::make_tuple(x, y, tz + 1);
                                auto minx = std::get<0>(tzminmax[tz + 1]);
                                auto miny = std::get<1>(tzminmax[tz + 1]);
                                auto maxx = std::get<2>(tzminmax[tz + 1]);
                                auto maxy = std::get<3>(tzminmax[tz + 1]);
                                size_t tileposx = 0;
                                size_t tileposy = 0;

                                if (x >= minx && x <= maxx && y >= miny && y <= maxy) {
                                    if (!((ty == 0 || ty == 1) || (ty != 0 && (y % (2 * ty)) != 0))) {
                                        tileposy = tile_size;
                                    }
                                    if (tx != 0) {
                                        tileposx = x % (2 * tx) * tile_size;
                                    } else if (tx == 0 && x == 1) {
                                        tileposx = tile_size;
                                    }

                                    tile_pos_t tile_pos = std::make_tuple(tileposx, tileposy);
                                    tile_pyramid[tz][low_tile_id].push_back(std::make_pair(tile_id, tile_pos));
                                }
                            }
                        }
                    }
                }
            }
        }
    };
}// namespace FASTTILER
#endif//FASTGDAL2TILES_TILEINFO_H