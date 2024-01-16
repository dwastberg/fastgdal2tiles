//
// Created by Dag Wästberg on 2023-11-29.
//

#include "RasterContainer.h"
#include "TileInfo.h"
#include "fasttiler.h"
#include "tile_fns.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <tuple>


namespace nb = nanobind;
using namespace nb::literals;

int add(int a, int b) {
    return a + b;
}


bool render_tiles(const std::string &in_raster, std::string &outdir, size_t min_zoom, size_t max_zoom, const nb::list &td_list, const nb::list &tzminmax_list, bool resume) {

    std::cout << "render_tiles" << std::endl;

    FASTTILER::TileInfo ti(min_zoom, max_zoom, tzminmax_list, td_list);

    auto tiles_done = FASTTILER::render_tiles(in_raster, ti, outdir, resume);

    if (!tiles_done) {
        std::cout << "failed to render tiles";
        return false;
    }
    // auto overview_tiles_done = FASTTILER::render_overview_tiles(in_raster, td_map, tile_pyramid, outdir);
    return true;
}

NB_MODULE(_fastgdal2tiles, m) {
    m.def("add", &add);
    m.def("render_tiles", &render_tiles);
}