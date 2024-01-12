//
// Created by Dag Wästberg on 2024-01-09.
//

#include "tile_fns.h"
td_map_t build_td_map(const td_vec_t &td_vec) {
    td_map_t td_map;
    for (const auto &td: td_vec) {
        td_map[std::make_tuple(td.tx, td.ty, td.tz)] = td;
    }
    return td_map;
}

td_vec_t convert_tile_details(const nb::list &td_list) {
    td_vec_t td_vec;
    for (const auto &pytd: td_list) {
        auto td = FASTTILER::tile_details();
        td.querysize = nb::cast<size_t>(pytd.attr("querysize"));
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

tzminmax_t convert_tzminmax(const nb::list &tzminmax_list) {
    tzminmax_t tzminmax;
    for (const auto &pytzminmax: tzminmax_list) {
        auto tzminmax_tuple = std::make_tuple(nb::cast<size_t>(pytzminmax[0]), nb::cast<size_t>(pytzminmax[1]), nb::cast<size_t>(pytzminmax[2]), nb::cast<size_t>(pytzminmax[3]));
        tzminmax.push_back(tzminmax_tuple);
    }
    return tzminmax;
}

tile_pyramid_t build_tile_pyramid(const size_t min_zoom, const size_t max_zoom, const tzminmax_t &tzminmax) {
    constexpr size_t tile_size = 256;
    tile_pyramid_t tile_pyramid;
    for (size_t tz = max_zoom - 1; tz >= min_zoom; tz--) {
        size_t tminx = std::get<0>(tzminmax[tz]);
        size_t tminy = std::get<1>(tzminmax[tz]);
        size_t tmaxx = std::get<2>(tzminmax[tz]);
        size_t tmaxy = std::get<3>(tzminmax[tz]);
        for (size_t ty = tmaxy; ty >= tminy; ty--) {
            for (size_t tx = tminx; tx <= tmaxx; tx++) {
                auto low_tile_id = std::make_tuple(tx, ty, tz);
                for (size_t y_off = 0; y_off < 2; y_off++) {
                    for (size_t x_off = 0; x_off < 2; x_off++) {
                        auto y = ty * 2 + y_off;
                        auto x = tx * 2 + x_off;
                        tile_id_t tile_id = std::make_tuple(x, y, tz + 1);
                        auto minx = std::get<0>(tzminmax[tz + 1]);
                        auto miny = std::get<1>(tzminmax[tz + 1]);
                        auto maxx = std::get<2>(tzminmax[tz + 1]);
                        auto maxy = std::get<3>(tzminmax[tz + 1]);
                        size_t tileposx = 0;
                        size_t tileposy = 0;

                        if (x >= minx && x <= maxx && y >= miny && y <= maxy) {
                            if (!((ty == 0 || ty == 1) || (ty != 0 && (y % (2 * ty)) != 0))) {
                                tileposy = tile_size;
                            }
                            if (tx != 0) {
                                tileposx = x % (2 * tx) * tile_size;
                            } else if (tx == 0 && x == 1) {
                                tileposx = tile_size;
                            }
                        }
                        tile_pos_t tile_pos = std::make_tuple(tileposx, tileposy);
                        tile_pyramid[tz][low_tile_id].push_back(std::make_pair(tile_id, tile_pos));
                    }
                }
            }
        }
    }
    return tile_pyramid;
}
