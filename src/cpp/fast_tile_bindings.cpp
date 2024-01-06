//
// Created by Dag Wästberg on 2023-11-29.
//

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <RasterContainer.h>
#include <fasttiler.h>

namespace nb = nanobind;
using namespace nb::literals;

int add(int a, int b) {
    return a + b;
}

std::vector<FASTTILER::tile_details> convert_tile_details(const nb::list &td_list)
{
    std::vector<FASTTILER::tile_details> td_vec;
    for (const auto &pytd : td_list)
    {
        auto td = FASTTILER::tile_details();
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

bool render_tiles(const std::string &in_raster, std::string &outdir, size_t min_zoom, size_t max_zoom, const nb::list &td_list)
{

    std::cout << "render_tiles" << std::endl;
    auto tiles_details = convert_tile_details(td_list);
    auto base_tiles_done = FASTTILER::render_basetiles(in_raster, tiles_details, outdir);
    if (!base_tiles_done)
    {
        std::cout << "failed to render basetiles";
        return false;
    }
    return true;
}

NB_MODULE(_fastgdal2tiles,m){
    m.def("add", &add);
    m.def("render_tiles",&render_tiles);
}