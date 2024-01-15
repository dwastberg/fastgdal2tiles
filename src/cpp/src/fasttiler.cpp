// Created by Dag Wästberg on 2023-11-29.
//

#include "fasttiler.h"
#include "BS_thread_pool.hpp"
#include "RasterContainer.h"
#include "image_processing.h"
#include "png_io.h"

#define BANDS 4

#include "logging/Timer.h"


namespace FASTTILER {

    bool render_tile(const RasterContainer *rc, const tile_details td, std::string output_dir) {
        auto tile = rc->get_subtile(td, BANDS);
        if (tile.size() == 0)
            return false;
        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
        PNG_IO::write_png_file(output_dir.c_str(), td.wxsize, td.wysize, BANDS, tile);
        return true;
    }

    void render_tile_from_threadpool(const std::vector<RasterContainer> &rc_list, const tile_details td, std::string output_dir, BS::thread_pool *write_pool) {
        auto thread_nr = BS::this_thread::get_index();
        // auto render_pool = BS::this_thread::get_pool().value();

        auto rc = &rc_list[thread_nr.value()];
        const auto tile_data = rc->get_subtile(td, BANDS);


        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
        if (tile_data.size() == 0)
            return;
        if (td.wxsize == td.querysize && td.wysize == td.querysize) {
            write_pool->detach_task([tile_data, td, output_dir]() {
                PNG_IO::write_tile(tile_data, td.querysize, td.querysize, BANDS, output_dir);
            });
            return;
        } else {
            write_pool->detach_task([tile_data, td, output_dir]() {
                PNG_IO::write_tile(tile_data, td.wxsize, td.wysize, td.querysize, td.querysize, td.wx, td.wy, BANDS, output_dir);
            });
        }

        //render_tile(rc, td, output_dir);
    }

    void render_overview_tile(const tile_id_t &overview_tile, const overview_tile_parts_t &tile_parts, std::string output_dir) {
        constexpr size_t tile_size = 256;

        auto tx = std::get<0>(overview_tile);
        auto ty = std::get<1>(overview_tile);
        auto tz = std::get<2>(overview_tile);


        std::vector<uint8_t> overview_tile_data((tile_size * tile_size * 4 * BANDS));
        std::vector<uint8_t> subtile_data(tile_size * tile_size * BANDS);
        std::vector<uint8_t> resized_tile_data(tile_size * tile_size * BANDS);
        DTCC::Timer build_tile_timer("build_overview_tile");
        for (const auto &tile_part: tile_parts) {
            auto subtile_id = tile_part.first;
            auto subtile_pos = tile_part.second;

            auto sub_tx = std::get<0>(subtile_id);
            auto sub_ty = std::get<1>(subtile_id);
            auto sub_tz = std::get<2>(subtile_id);


            auto filename = output_dir + "/" + std::to_string(sub_tz) + "/" + std::to_string(sub_tx) + "/" + std::to_string(sub_ty) + ".png";
            auto tile_read_sucess = PNG_IO::read_png_file(filename.c_str(), tile_size, tile_size, BANDS, subtile_data);
            if (!tile_read_sucess) {
                std::cout << "failed to read " << filename << std::endl;
                continue;
            }
            auto subtile_offset = (subtile_pos.first * BANDS) + (subtile_pos.second * (tile_size * 2) * BANDS);
            for (size_t y = 0; y < tile_size; y++) {
                auto subtile_start = (y * tile_size * BANDS);
                auto subtile_end = subtile_start + (tile_size * BANDS);
                auto overview_start = subtile_offset + (y * tile_size * 2 * BANDS);
                std::copy(subtile_data.begin() + subtile_start, subtile_data.begin() + subtile_end, overview_tile_data.begin() + overview_start);
            }
        }
        build_tile_timer.stop();

        DTCC::Timer shrink_tile_timer("shrink_overview_tile");
        FASTTILER::IMAGE_PROCESSING::shrink_tile(overview_tile_data, tile_size * 2, BANDS, tile_size, resized_tile_data);
        shrink_tile_timer.stop();
        DTCC::Timer write_tile_timer("write_overview_tile");
        output_dir.append("/").append(std::to_string(tz)).append("/").append(std::to_string(tx)).append("/").append(std::to_string(ty)).append(".png");
        write_tile_timer.stop();
        PNG_IO::write_png_file(output_dir.c_str(), tile_size, tile_size, BANDS, resized_tile_data);
    }

    bool render_overview_tiles(size_t tz, const tile_pyramid_t &tile_pyramid, std::string outdir) {

        auto overview_tiles = tile_pyramid.at(tz);
        std::cout << "rendering overview tiles for zoom level " << tz << std::endl;
        std::cout << "a total of " << overview_tiles.size() << " overview tiles" << std::endl;
        for (const auto &kv: overview_tiles) {
            auto tile_id = kv.first;
            auto tile_parts = kv.second;
            render_overview_tile(tile_id, tile_parts, outdir);
        }
        return true;
    }


    bool render_basetiles(std::string in_raster, const std::vector<tile_details> &tile_list, std::string out_dir) {

        constexpr float render_pool_ratio = 0.6;
        size_t total_thread_count = std::thread::hardware_concurrency();
        size_t render_pool_size = (size_t) round(total_thread_count * render_pool_ratio);
        size_t write_pool_size = total_thread_count - render_pool_size;
        if (write_pool_size == 0)
            write_pool_size = 1;
        std::cout << "render pool: " << std::to_string(render_pool_size) << " write pool: " << std::to_string(write_pool_size) << std::endl;

        BS::thread_pool render_pool(render_pool_size);
        BS::thread_pool write_pool(write_pool_size);

        const auto tile_count = tile_list.size();
        const auto render_thread_count = render_pool.get_thread_count();

        // each thread gets its own RasterContainer to avoid
        // locking when reading from the same raster
        std::vector<RasterContainer> rc_arr(render_thread_count);
        for (size_t i = 0; i < render_thread_count; i++) {
            rc_arr[i].set_raster(in_raster);
        }

        for (const auto &td: tile_list) {
            // auto rc = &rc_arr[count % thread_count];
            // render_tile(rc, td, out_dir);
            render_pool.detach_task([&, out_dir]() { render_tile_from_threadpool(rc_arr, td, out_dir, &write_pool); });
        }
        render_pool.wait();
        std::cout << "render pool done!" << std::endl;
        write_pool.wait();
        return true;
    }

    bool render_tiles(std::string in_raster, size_t min_zoom, size_t max_zoom, const std::vector<tile_details> &tile_list, const tile_pyramid_t &tile_pyramid, std::string out_dir) {
        fpng::fpng_init();

        DTCC::Timer basetile_timer("basetiles");
        auto basetiles_done = render_basetiles(in_raster, tile_list, out_dir);
        if (!basetiles_done) {
            std::cout << "failed to render basetiles";
            return false;
        }
        basetile_timer.stop();
        DTCC::Timer overview_timer("overview");
        for (size_t tz = max_zoom - 1; tz >= min_zoom; tz--) {
            auto overview_tiles_done = render_overview_tiles(tz, tile_pyramid, out_dir);
            if (!overview_tiles_done) {
                std::cout << "failed to render overview tiles";
                return false;
            }
        }
        overview_timer.stop();
        DTCC::Timer::report("render_tiles");
        return true;
    }

}// namespace FASTTILER
