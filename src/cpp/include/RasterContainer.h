//
// Created by Dag Wästberg on 2023-11-29.
//

#ifndef FASTGDAL2TILES_RASTERCONTAINER_H
#define FASTGDAL2TILES_RASTERCONTAINER_H

#include <gdal_priv.h>
#include <iostream>
#include <mutex>
#include <vector>

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

    class RasterContainer {
        // mutable std::mutex read_raster_mutex;

    public:
        bool hasAlpha;
        RasterContainer() : in_raster(nullptr) {
            GDALAllRegister();
        }

        RasterContainer(const std::string &filename) {
            auto c_filename = filename.c_str();
            GDALAllRegister();
            in_raster = (GDALDataset *) GDALOpen(c_filename, GA_ReadOnly);
            if (in_raster == nullptr) {
                throw std::runtime_error("Could not open file");
            }
            hasAlpha = in_raster->GetRasterCount() == 4;
        }
        bool set_raster(const std::string &filename) {
            auto c_filename = filename.c_str();
            in_raster = (GDALDataset *) GDALOpen(c_filename, GA_ReadOnly);
            if (in_raster == nullptr) {
                return false;
            }
            hasAlpha = in_raster->GetRasterCount() == 4;
            return true;
        }
        //        ~RasterContainer()
        //        {
        //            GDALClose(in_raster);
        //        }

        std::vector<uint8_t> get_subtile(const tile_details &tile, size_t bands) const {
            const size_t size = tile.wxsize * tile.wysize;
            std::vector<uint8_t> rgbData(size * bands,255);

            // Read data directly into rgbData
            {
                auto err1 = in_raster->GetRasterBand(1)->RasterIO(GF_Read, tile.rx, tile.ry, tile.rxsize, tile.rysize, rgbData.data() + 0, tile.wxsize, tile.wysize, GDT_Byte, bands, bands * tile.wxsize);
                auto err2 = in_raster->GetRasterBand(2)->RasterIO(GF_Read, tile.rx, tile.ry, tile.rxsize, tile.rysize, rgbData.data() + 1, tile.wxsize, tile.wysize, GDT_Byte, bands, bands * tile.wxsize);
                auto err3 = in_raster->GetRasterBand(3)->RasterIO(GF_Read, tile.rx, tile.ry, tile.rxsize, tile.rysize, rgbData.data() + 2, tile.wxsize, tile.wysize, GDT_Byte, bands, bands * tile.wxsize);
                if (err1 != 0 || err2 != 0 || err3 != 0)
                    rgbData.clear();
            }


            return rgbData;
        }

    private:
        GDALDataset *in_raster;
    };

}// namespace FASTTILER

#endif// FASTGDAL2TILES_RASTERCONTAINER_H
