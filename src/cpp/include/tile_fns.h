//
// Created by Dag Wästberg on 2024-01-09.
//

#ifndef FASTGDAL2TILES_TILE_FNS_H
#define FASTGDAL2TILES_TILE_FNS_H

#include "RasterContainer.h"
#include <tuple>
#include <unordered_map>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;


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
typedef std::tuple<size_t, size_t> tile_pos_t;
typedef std::unordered_map<size_t, std::unordered_map<tile_id_t, std::vector<std::pair<tile_id_t, tile_pos_t>>, TileIDHash>> tile_pyramid_t;
typedef std::vector<std::tuple<size_t, size_t, size_t, size_t>> tzminmax_t;

td_map_t build_td_map(const td_vec_t &td_vec);
td_vec_t convert_tile_details(const nb::list &td_list);
tzminmax_t convert_tzminmax(const nb::list &tzminmax_list);
tile_pyramid_t build_tile_pyramid(const size_t min_zoom, const size_t max_zoom, const tzminmax_t &tzminmax);

#endif//FASTGDAL2TILES_TILE_FNS_H
