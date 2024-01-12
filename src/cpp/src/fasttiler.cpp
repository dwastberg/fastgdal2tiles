// Created by Dag Wästberg on 2023-11-29.
//

#include "fasttiler.h"
#include "BS_thread_pool.hpp"
#include "RasterContainer.h"
#include "png_io.h"

namespace FASTTILER {

    bool render_tile(const RasterContainer *rc, const tile_details td, std::string output_dir) {
        auto tile = rc->get_subtile(td);
        if (tile.size() == 0)
            return false;
        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
        PNG_IO::write_png_file(output_dir.c_str(), td.wxsize, td.wysize, 3, tile);
        return true;
    }

    bool write_tile(const std::vector<uint8_t> &img_data, size_t width, size_t height, size_t bands, const std::string &out_path) {
        // std::cout << "writing " << out_path << std::endl;
        return PNG_IO::write_png_file(out_path.c_str(), width, height, bands, img_data);
    }

    bool write_tile(const std::vector<uint8_t> &img_data, size_t in_width, size_t in_height, size_t out_width, size_t out_height, size_t x_offset, size_t y_offset, size_t bands, const std::string &out_path)
    {
        std::vector<uint8_t> offset_img(out_width*out_height*bands);

        const size_t initial_offset = ((y_offset * out_width) + x_offset) * bands;

        for (size_t y = 0; y < in_height; y++)
        {
            size_t in_img_start = (y * in_width * bands);
            size_t in_img_end = in_img_start + (in_width * bands);
            size_t out_img_start = initial_offset + (y * out_width * bands);
            std::copy(img_data.begin() + in_img_start, img_data.begin() + in_img_end, offset_img.begin() + out_img_start);
        }

        return PNG_IO::write_png_file(out_path.c_str(), out_width, out_height, bands, offset_img);

    }

    void render_tile_from_threadpool(const std::vector<RasterContainer> &rc_list, const tile_details td, std::string output_dir, BS::thread_pool *write_pool) {
        auto thread_nr = BS::this_thread::get_index();
        // auto render_pool = BS::this_thread::get_pool().value();

        auto rc = &rc_list[thread_nr.value()];
        const auto tile_data = rc->get_subtile(td);
        output_dir.append("/").append(std::to_string(td.tz)).append("/").append(std::to_string(td.tx)).append("/").append(std::to_string(td.ty)).append(".png");
        if (tile_data.size() == 0)
            return;
        if (td.wxsize == td.querysize && td.wysize == td.querysize) {
            write_pool->detach_task([tile_data, td, output_dir]() {
                write_tile(tile_data, td.querysize, td.querysize, 3, output_dir);
            });
            return;
        }
        else {
            write_pool->detach_task([tile_data, td, output_dir]() {
                write_tile(tile_data, td.wxsize, td.wysize, td.querysize, td.querysize, td.wx, td.wy, 3, output_dir);
            });
        }

        //render_tile(rc, td, output_dir);
    }


    bool render_basetiles(std::string in_raster, const std::vector<tile_details> &tile_list, std::string out_dir) {
        fpng::fpng_init();
        // BS::thread_pool pool;
        constexpr float render_pool_ratio = 0.5;
        size_t total_thread_count = std::thread::hardware_concurrency();
        size_t render_pool_size = (size_t) round(total_thread_count*render_pool_ratio);
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
            render_pool.detach_task([&]() { render_tile_from_threadpool(rc_arr, td, out_dir, &write_pool); });
        }
        render_pool.wait();
        std::cout << "render pool done!";
        write_pool.wait();
        return true;
    }

    bool render_tile_pyramid(std::string in_raster, const td_map_t &td_map, const tile_pyramid_t &tile_pyramid, std::string out_dir)
    {
        constexpr float render_pool_ratio = 0.8;
        size_t thread_count = std::thread::hardware_concurrency();
        size_t render_pool_size = (size_t) round(thread_count*render_pool_ratio);
        size_t write_pool_size = thread_count - render_pool_size;
        if (write_pool_size == 0)
            write_pool_size = 1;
        BS::thread_pool render_pool(render_pool_size);
        BS::thread_pool write_pool(write_pool_size);

        // each thread gets its own RasterContainer to avoid
        // locking when reading from the same raster
        std::vector<RasterContainer> rc_arr(thread_count);
        for (size_t i = 0; i < thread_count; i++) {
            rc_arr[i].set_raster(in_raster);
        }


        return true;
    }

}// namespace FASTTILER
