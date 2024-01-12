//
// Created by Dag Wästberg on 2023-11-29.
//

#include <RasterContainer.h>
#include <fasttiler.h>
#include "tile_fns.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <tuple>


namespace nb = nanobind;
using namespace nb::literals;

int add(int a, int b) {
    return a + b;
}


bool render_tiles(const std::string &in_raster, std::string &outdir, size_t min_zoom, size_t max_zoom, const nb::list &td_list, const nb::list &tzminmax_list) {

    std::cout << "render_tiles" << std::endl;
    auto tiles_details = convert_tile_details(td_list);
    auto td_map = build_td_map(tiles_details);
    auto tzminmax = convert_tzminmax(tzminmax_list);
    auto tile_pyramid = build_tile_pyramid(min_zoom, max_zoom, tzminmax);
//    std::cout << "tile_pyramid size: " << tile_pyramid.size() << std::endl;
//    std::cout << "tile_pyramid 21 size: " << tile_pyramid[21].size() << std::endl;
//    std::cout << "tile_pyramid 20 size: " << tile_pyramid[20].size() << std::endl;
//    for (const auto &kv: tile_pyramid[16]) {
//        std::cout << "tile_pyramid 16: " << std::get<0>(kv.first) << " " << std::get<1>(kv.first) << " " << std::get<2>(kv.first);
//        std::cout << " " << std::get<0>(kv.second[0].first);
//        std::cout << " " << std::get<1>(kv.second[0].first);
//        std::cout << " " << std::get<2>(kv.second[0].first);
//        std::cout << " " << std::get<0>(kv.second[0].second);
//        std::cout << " " << std::get<1>(kv.second[0].second);
//        std::cout << std::endl;
//    }
    auto base_tiles_done = FASTTILER::render_basetiles(in_raster, tiles_details, outdir);
    if (!base_tiles_done) {
        std::cout << "failed to render basetiles";
        return false;
    }
    return true;
}

NB_MODULE(_fastgdal2tiles, m) {
    m.def("add", &add);
    m.def("render_tiles", &render_tiles);
}