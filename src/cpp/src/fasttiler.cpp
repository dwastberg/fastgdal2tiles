// Created by Dag Wästberg on 2023-11-29.
//

#include "fasttiler.h"
#include "BS_thread_pool.hpp"
#include "RasterContainer.h"
#include "png_io.h"

namespace FASTTILER {

    bool render_tile(const RasterContainer &rc, const tile_details td, std::string output_dir) {
        auto tile = rc.get_subtile(td);
        if (tile.size() == 0)
            return false;
        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
        PNG_IO::write_png_file(output_dir.c_str(), td.wxsize, td.wysize, 3, tile);
        return true;
    }

    bool render_basetiles(const RasterContainer &rc,const std::vector<tile_details> &tile_list, std::string out_dir )
    {
        for(const auto &td: tile_list )
        {
            render_tile(rc, td, out_dir);
        }
        return true;
    }

    bool create_tiles(const std::string file_name, const std::string output_dir, const std::vector<tile_details> &tile_job)

    {
        const auto rc = RasterContainer(file_name.c_str());
        const auto tile_count = tile_job.size();
        BS::thread_pool pool;

        return true;
    }
}



