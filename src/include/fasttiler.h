//
// Created by Dag Wästberg on 2023-11-29.
//

#ifndef FASTGDAL2TILES_FASTTILER_H
#define FASTGDAL2TILES_FASTTILER_H

#include <filesystem>

#include "RasterContainer.h"
#include "TileInfo.h"

namespace FASTTILER {
    // bool render_tile(const RasterContainer &rc, const tile_details td, std::string output_dir);
    bool render_tiles(std::string in_raster,const TileInfo &tile_info, std::string out_dir, bool resume, bool progress_bar, float render_pool_ratio);
    // bool render_basetiles(std::string in_raster,const std::vector<tile_details> &tile_list, const std::filesystem::path &out_dir_path, bool resume);
    // bool render_tile_pyramid(std::string in_raster, const td_map_t &td_map, const tile_pyramid_t &tile_pyramid, std::string out_dir);

}

#endif //FASTGDAL2TILES_FASTTILER_H
