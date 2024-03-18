//
// Created by Dag Wästberg on 2024-01-15.
//

#ifndef FASTGDAL2TILES_TILEINFO_PY_H
#define FASTGDAL2TILES_TILEINFO_PY_H

#include "RasterContainer.h"
#include <tuple>
#include <unordered_map>

#include <nanobind/nanobind.h>
namespace nb = nanobind;

namespace FASTTILER {
    typedef std::tuple<size_t, size_t, size_t> tile_id_t;

    struct TileIDHash {
        std::size_t operator()(const tile_id_t &t) const {
            std::hash<int> hasher;
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


    class TileInfo_py {
    public:
        td_vec_t td_vec;
        td_map_t td_map;
        tile_pyramid_t tile_pyramid;
        tzminmax_t tzminmax;
        size_t min_zoom;
        size_t max_zoom;
        std::string in_raster;
        std::string outdir;
        TileInfo_py(size_t min_zoom, size_t max_zoom, const nb::list &tzminmax_list, const nb::list &td_list) {
            this->min_zoom = min_zoom;
            this->max_zoom = max_zoom;
            convert_tzminmax(tzminmax_list);
            convert_tile_details(td_list);
            build_td_map(td_vec);
            build_tile_pyramid(min_zoom, max_zoom, tzminmax);
        }

    private:
        td_vec_t convert_tile_details(const nb::list &td_list) {
            for (const auto &pytd: td_list) {
                tile_details td;
                td.querysize = nb::cast<size_t>(pytd.attr("querysize"));
                td.rx = nb::cast<size_t>(pytd.attr("rx"));
                td.ry = nb::cast<size_t>(pytd.attr("ry"));
                td.rxsize = nb::cast<size_t>(pytd.attr("rxsize"));
                td.rysize = nb::cast<size_t>(pytd.attr("rysize"));
                td.tx = nb::cast<size_t>(pytd.attr("tx"));
                td.ty = nb::cast<size_t>(pytd.attr("ty"));
                td.tz = nb::cast<size_t>(pytd.attr("tz"));
                td.wx = nb::cast<size_t>(pytd.attr("wx"));
                td.wy = nb::cast<size_t>(pytd.attr("wy"));
                td.wxsize = nb::cast<size_t>(pytd.attr("wxsize"));
                td.wysize = nb::cast<size_t>(pytd.attr("wysize"));
                td_vec.push_back(td);
            }
            return td_vec;
        }

        tzminmax_t convert_tzminmax(const nb::list &tzminmax_list) {
            for (const auto &pytzminmax: tzminmax_list) {
                auto tzminmax_tuple = std::make_tuple(nb::cast<size_t>(pytzminmax[0]), nb::cast<size_t>(pytzminmax[1]), nb::cast<size_t>(pytzminmax[2]), nb::cast<size_t>(pytzminmax[3]));
                tzminmax.push_back(tzminmax_tuple);
            }
            return tzminmax;
        }

        td_map_t build_td_map(const td_vec_t &td_vec) {
            for (const auto &td: td_vec) {
                td_map[std::make_tuple(td.tx, td.ty, td.tz)] = td;
            }
            return td_map;
        }
        tile_pyramid_t build_tile_pyramid(const size_t min_zoom, const size_t max_zoom, const tzminmax_t &tzminmax) {
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
            return tile_pyramid;
        }
    };

}// namespace FASTTILER
#endif//FASTGDAL2TILES_TILEINFO_PY_H
