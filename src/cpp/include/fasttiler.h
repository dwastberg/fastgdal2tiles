//
// Created by Dag Wästberg on 2023-11-29.
//

#ifndef FASTGDAL2TILES_FASTTILER_H
#define FASTGDAL2TILES_FASTTILER_H

#include <filesystem>

#include "RasterContainer.h"


namespace FASTTILER {
    bool create_tiles(const std::string file_name, const std::string output_dir, const std::vector<tile_details> &tile_job);
    bool render_tile(const RasterContainer &rc, const tile_details td, std::string output_dir);
    bool render_basetiles(const RasterContainer &rc,const std::vector<tile_details> &tile_list, std::string out_dir);

}

#endif //FASTGDAL2TILES_FASTTILER_H
