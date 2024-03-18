//
// Created by Dag Wästberg on 2024-03-06.
//

#ifndef FASTGDAL2TILES_RASTER_INFO_H
#define FASTGDAL2TILES_RASTER_INFO_H

#include "GlobalMercator.h"
#include <gdal_priv.h>


#include <iostream>
#include <filesystem>
#include <tuple>

#include "raster_info.h"

#define MAX_ZOOM 32

typedef std::vector<std::tuple<size_t, size_t, size_t, size_t>> tzminmax_t;
typedef std::vector<std::tuple<size_t, size_t, size_t, size_t>> obounds_t;

std::tuple<size_t, size_t, size_t> raster_size(const std::filesystem::path &filename) {
    GDALAllRegister();
    std::string filename_str = filename;
    auto poDataset = (GDALDataset *) GDALOpen(filename_str.c_str(), GA_ReadOnly);
    if (poDataset == NULL) {
        std::cerr << "Could not open " << filename << std::endl;
        exit(1);
    }
    size_t xsize = poDataset->GetRasterXSize();
    size_t ysize = poDataset->GetRasterYSize();
    size_t bands = poDataset->GetRasterCount();
    GDALClose(poDataset);
    return std::make_tuple(xsize, ysize, bands);
}




#endif//FASTGDAL2TILES_RASTER_INFO_H
