// Created by Dag Wästberg on 2023-11-29.
//

#include "fasttiler.h"
#include "TileInfo.h"

#include "RasterContainer.h"
#include "image_processing.h"
#include "png_io.h"

#define BANDS 4

#include "BS_thread_pool.hpp"
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>

#include <filesystem>


namespace FASTTILER {

    //    bool render_tile(const RasterContainer *rc, const tile_details td, std::string output_dir) {
    //        auto tile = rc->get_subtile(td, BANDS);
    //        if (tile.size() == 0)
    //            return false;
    //        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
    //        PNG_IO::write_png_file(output_dir.c_str(), td.wxsize, td.wysize, BANDS, tile);
    //        return true;
    //    }

    void render_tile_from_threadpool(const std::vector<RasterContainer> &rc_list, const tile_details td, const std::filesystem::path &img_path, BS::thread_pool *write_pool) {
        auto thread_nr = BS::this_thread::get_index();
        // auto render_pool = BS::this_thread::get_pool().value();

        auto rc = &rc_list[thread_nr.value()];
        const auto tile_data = rc->get_subtile(td, BANDS);


        if (tile_data.size() == 0)
            return;
        std::filesystem::path out_dir_path = img_path.parent_path();
        std::filesystem::create_directories(out_dir_path);
        if (td.wxsize == td.querysize && td.wysize == td.querysize) {
            write_pool->detach_task([tile_data, td, img_path]() {
                PNG_IO::write_tile(tile_data, td.querysize, td.querysize, BANDS, img_path.string());
            });
            return;
        } else {
            write_pool->detach_task([tile_data, td, img_path]() {
                PNG_IO::write_tile(tile_data, td.wxsize, td.wysize, td.querysize, td.querysize, td.wx, td.wy, BANDS, img_path.string());
            });
        }

        //render_tile(rc, td, output_dir);
    }

    void render_overview_tile(const overview_tile_parts_t &tile_parts, const std::filesystem::path &output_dir, const std::filesystem::path &out_img_path) {
        constexpr size_t tile_size = 256;

        std::vector<uint8_t> overview_tile_data((tile_size * tile_size * 4 * BANDS));
        std::vector<uint8_t> subtile_data(tile_size * tile_size * BANDS);
        std::vector<uint8_t> resized_tile_data(tile_size * tile_size * BANDS);
        for (const auto &tile_part: tile_parts) {
            auto subtile_id = tile_part.first;
            auto subtile_pos = tile_part.second;

            auto sub_tx = std::get<0>(subtile_id);
            auto sub_ty = std::get<1>(subtile_id);
            auto sub_tz = std::get<2>(subtile_id);


            auto img_path = output_dir / std::to_string(sub_tz) / std::to_string(sub_tx) / (std::to_string(sub_ty) + ".png");
            auto tile_read_sucess = PNG_IO::read_png_file(img_path.string().c_str(), tile_size, tile_size, BANDS, subtile_data);
            if (!tile_read_sucess) {
                std::cout << "failed to read " << img_path.string() << std::endl;
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

        FASTTILER::IMAGE_PROCESSING::shrink_tile(overview_tile_data, tile_size * 2, BANDS, tile_size, resized_tile_data);
        PNG_IO::write_png_file(out_img_path.string().c_str(), tile_size, tile_size, BANDS, resized_tile_data);
    }

    bool render_overview_tiles(size_t tz, const tile_pyramid_t &tile_pyramid, const std::filesystem::path &outdir, bool resume = false, bool progress_bar = true) {

        BS::thread_pool render_pool(std::thread::hardware_concurrency());
        auto overview_tiles = tile_pyramid.at(tz);
        //        std::cout << "rendering overview tiles for zoom level " << tz << std::endl;
        //        std::cout << "a total of " << overview_tiles.size() << " overview tiles" << std::endl;
        const size_t tile_count = overview_tiles.size();
        size_t skipped_tiles = 0;
        for (const auto &kv: overview_tiles) {
            auto tile_id = kv.first;
            auto tile_parts = kv.second;
            auto tx = std::get<0>(tile_id);
            auto ty = std::get<1>(tile_id);
            auto tz = std::get<2>(tile_id);

            auto img_path = outdir / std::to_string(tz) / std::to_string(tx) / (std::to_string(ty) + ".png");
            std::filesystem::create_directories(img_path.parent_path());
            if (resume && std::filesystem::exists(img_path)) {
                skipped_tiles++;
                continue;
            }

            render_pool.detach_task([tile_parts, outdir, img_path]() { render_overview_tile(tile_parts, outdir, img_path); });
            // render_overview_tile(tile_id, tile_parts, outdir);
        }
        if (progress_bar) {
            indicators::ProgressBar bar{
                    indicators::option::BarWidth{50},
                    indicators::option::Start{"["},
                    indicators::option::Fill{"="},
                    indicators::option::Lead{">"},
                    indicators::option::Remainder{"."},
                    indicators::option::End{"]"},
                    indicators::option::PostfixText{"Rendering z" + std::to_string(tz) + " overview tiles"},
                    indicators::option::ShowPercentage{true},
                    //                indicators::option::ForegroundColor{indicators::Color::green},
                    //                indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
            };

            float tiles_todo_count = tile_count - skipped_tiles;
            indicators::show_console_cursor(false);
            while (render_pool.get_tasks_total() > 0) {
                int pct_done = static_cast<int>(((tiles_todo_count - render_pool.get_tasks_total()) / tile_count) * 100);
                bar.set_progress(pct_done);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            bar.set_progress(100);
            indicators::show_console_cursor(true);
        }
        render_pool.wait();
        return true;
    }


    bool render_basetiles(std::string in_raster, const std::vector<tile_details> &tile_list, const std::filesystem::path &out_dir_path, bool resume = false, bool progress_bar = true, float render_pool_ratio = 0.6) {
        // constexpr float render_pool_ratio = 0.6;
        size_t total_thread_count = std::thread::hardware_concurrency();
        size_t render_pool_size = (size_t) round(total_thread_count * render_pool_ratio);
        size_t tz = tile_list[0].tz;
        size_t write_pool_size = total_thread_count - render_pool_size;
        if (write_pool_size == 0)
            write_pool_size = 1;
        write_pool_size *= 1.5; // write pool IO means we get slightly higer performance with more threads

        // std::cout << "render pool: " << std::to_string(render_pool_size) << " write pool: " << std::to_string(write_pool_size) << std::endl;

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
        size_t skipped_tiles = 0;
        for (const auto &td: tile_list) {
            // auto rc = &rc_arr[count % thread_count];
            // render_tile(rc, td, out_dir);
            auto img_path = out_dir_path / std::to_string(td.tz) / std::to_string(td.tx) / (std::to_string(td.ty) + ".png");
            if (resume && std::filesystem::exists(img_path)) {
                skipped_tiles++;
                continue;
            }
            render_pool.detach_task([&, img_path]() { render_tile_from_threadpool(rc_arr, td, img_path, &write_pool); });
        }
        if (progress_bar) {
            indicators::ProgressBar bar{
                    indicators::option::BarWidth{50},
                    indicators::option::Start{"["},
                    indicators::option::Fill{"="},
                    indicators::option::Lead{">"},
                    indicators::option::Remainder{"."},
                    indicators::option::End{"]"},
                    indicators::option::PostfixText{"Rendering z" + std::to_string(tz) + " basetiles"},
                    indicators::option::ShowPercentage{true}
                    //                indicators::option::ForegroundColor{indicators::Color::green},
                    //                indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
            };

            float tiles_todo_count = tile_count - skipped_tiles;
            indicators::show_console_cursor(false);
            while (render_pool.get_tasks_total() > 0) {
                int pct_done = static_cast<int>(((tiles_todo_count - render_pool.get_tasks_total()) / tile_count) * 100);
                bar.set_progress(pct_done);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            bar.set_progress(100);
            indicators::show_console_cursor(true);
        }
        render_pool.wait();
        // std::cout << "render pool done!" << std::endl;
        write_pool.wait();
        return true;
    }

    bool render_top_N_levels(std::string in_raster, size_t n, const TileInfo &tile_info, std::filesystem::path out_dir, bool resume) {
        fpng::fpng_init();


        if (n == 1)
            return render_basetiles(in_raster, tile_info.td_vec, out_dir);
        constexpr float render_pool_ratio = 0.6;
        size_t total_thread_count = std::thread::hardware_concurrency();
        size_t render_pool_size = (size_t) round(total_thread_count * render_pool_ratio);
        size_t tz = tile_info.max_zoom;
        size_t write_pool_size = total_thread_count - render_pool_size;
        if (write_pool_size == 0)
            write_pool_size = 1;

        BS::thread_pool render_pool(render_pool_size);
        BS::thread_pool write_pool(write_pool_size);

        size_t top_z = tile_info.max_zoom;
        auto overview_tiles = tile_info.tile_pyramid.at(top_z - 1);
        std::vector<uint8_t> tile_0(256 * 256 * BANDS);
        std::vector<uint8_t> tile_1(256 * 256 * BANDS);
        std::vector<uint8_t> tile_2(256 * 256 * BANDS);
        std::vector<uint8_t> tile_3(256 * 256 * BANDS);
        std::vector<uint8_t> overview_tile(256 * 256 * 4 * BANDS);
        size_t skipped_tiles = 0;
        for (const auto &kv: overview_tiles) {
            auto tile_id = kv.first;
            auto tile_parts = kv.second;
            auto tx = std::get<0>(tile_id);
            auto ty = std::get<1>(tile_id);
            auto tz = std::get<2>(tile_id);

            auto img_path = out_dir / std::to_string(tz) / std::to_string(tx) / (std::to_string(ty) + ".png");
            if (resume && std::filesystem::exists(img_path)) {
                skipped_tiles++;
                continue;
            }

            render_pool.detach_task([tile_parts, out_dir, img_path]() { render_overview_tile(tile_parts, out_dir, img_path); });
            // render_overview_tile(tile_id, tile_parts, outdir);
        }
        return true;
    }

    bool render_tiles(std::string in_raster, const TileInfo &tile_info, std::string out_dir, bool resume, bool progress_bar, float render_pool_ratio) {
        fpng::fpng_init();
        //
        //        auto td_map = build_td_map(tile_list);

        const std::filesystem::path out_dir_path(out_dir);

        const std::vector<tile_details> tile_list = tile_info.td_vec;

        auto basetiles_done = render_basetiles(in_raster, tile_list, out_dir_path, resume, progress_bar, render_pool_ratio);
        if (!basetiles_done) {
            std::cout << "failed to render basetiles";
            return false;
        }
        for (size_t tz = tile_info.max_zoom - 1; tz >= tile_info.min_zoom; tz--) {
            auto overview_tiles_done = render_overview_tiles(tz, tile_info.tile_pyramid, out_dir_path, resume, progress_bar);
            if (!overview_tiles_done) {
                std::cout << "failed to render overview tiles";
                return false;
            }
        }
        return true;
    }

}// namespace FASTTILER
