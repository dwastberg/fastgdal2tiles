//
// Created by Dag Wästberg on 2024-03-06.
//

#ifndef FASTGDAL2TILES_GLOBALMERCATOR_H
#define FASTGDAL2TILES_GLOBALMERCATOR_H

#include <cmath>
#include <tuple>
#include <string>

class GlobalMercator {
public:
    int tileSize;
    double initialResolution;
    double originShift;

    GlobalMercator(int tileSize = 256) {
        this->tileSize = tileSize;
        this->initialResolution = 2 * M_PI * 6378137 / tileSize; // 156543.03392804062 for tileSize 256 pixels
        this->originShift = 2 * M_PI * 6378137 / 2.0; // 20037508.342789244
    }

    std::tuple<double, double> LatLonToMeters(double lat, double lon) {
        double mx = lon * this->originShift / 180.0;
        double my = std::log(std::tan((90 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
        my = my * this->originShift / 180.0;
        return std::make_tuple(mx, my);
    }

    std::tuple<double, double> MetersToLatLon(double mx, double my) {
        double lon = (mx / this->originShift) * 180.0;
        double lat = (my / this->originShift) * 180.0;
        lat = 180 / M_PI * (2 * std::atan(std::exp(lat * M_PI / 180.0)) - M_PI / 2.0);
        return std::make_tuple(lat, lon);
    }

    std::tuple<double, double> PixelsToMeters(double px, double py, int zoom) {
        double res = Resolution(zoom);
        double mx = px * res - this->originShift;
        double my = py * res - this->originShift;
        return std::make_tuple(mx, my);
    }

    std::tuple<double, double> MetersToPixels(double mx, double my, int zoom) {
        double res = Resolution(zoom);
        double px = (mx + this->originShift) / res;
        double py = (my + this->originShift) / res;
        return std::make_tuple(px, py);
    }

    std::tuple<size_t, size_t> PixelsToTile(double px, double py) {
        size_t tx = static_cast<size_t>(std::ceil(px / static_cast<double>(this->tileSize)) - 1);
        size_t ty = static_cast<size_t>(std::ceil(py / static_cast<double>(this->tileSize)) - 1);
        return std::make_tuple(tx, ty);
    }

    std::tuple<double, double> PixelsToRaster(double px, double py, int zoom) {
        size_t mapSize = this->tileSize << zoom;
        return std::make_tuple(px, mapSize - py);
    }

    std::tuple<size_t, size_t>  MetersToTile(double mx, double my, int zoom) {
        auto [px, py] = MetersToPixels(mx, my, zoom);
        return PixelsToTile(px, py);
    }

    std::tuple<double, double, double, double> TileBounds(int tx, int ty, int zoom) {
        auto [minx, miny] = PixelsToMeters(tx * this->tileSize, ty * this->tileSize, zoom);
        auto [maxx, maxy] = PixelsToMeters((tx + 1) * this->tileSize, (ty + 1) * this->tileSize, zoom);
        return std::make_tuple(minx, miny, maxx, maxy);
    }

    std::tuple<double, double, double, double> TileLatLonBounds(int tx, int ty, int zoom) {
        auto [minx, miny, maxx, maxy] = TileBounds(tx, ty, zoom);
        auto [minLat, minLon] = MetersToLatLon(minx, miny);
        auto [maxLat, maxLon] = MetersToLatLon(maxx, maxy);
        return std::make_tuple(minLat, minLon, maxLat, maxLon);
    }

    double Resolution(int zoom) {
        return this->initialResolution / std::pow(2.0, zoom);
    }

    int ZoomForPixelSize(double pixelSize) {
        for (int i = 0; i < 30; ++i) { // Assuming a MAXZOOMLEVEL of 30
            if (pixelSize > Resolution(i)) {
                return i != 0 ? i - 1 : 0;
            }
        }
        return 0; // Default to zoom level 0 if something goes wrong
    }

    std::tuple<size_t,size_t> GoogleTile(int tx, int ty, int zoom) {
        return std::make_tuple(tx, static_cast<size_t>(std::pow(2, zoom) - 1 - ty));
    }

    std::string QuadTree(int tx, int ty, int zoom) {
        std::string quadKey = "";
        ty = (1 << zoom) - 1 - ty;
        for (int i = zoom; i > 0; i--) {
            int digit = 0;
            int mask = 1 << (i - 1);
            if ((tx & mask) != 0) {
                digit += 1;
            }
            if ((ty & mask) != 0) {
                digit += 2;
            }
            quadKey += std::to_string(digit);
        }
        return quadKey;
    }
};


#endif//FASTGDAL2TILES_GLOBALMERCATOR_H
