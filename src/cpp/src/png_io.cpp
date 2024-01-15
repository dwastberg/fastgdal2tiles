//
// Created by Dag Wästberg on 2024-01-03.
//
#include "png_io.h"
#include <iostream>


namespace FASTTILER::PNG_IO {
    bool write_png_file(const char *file_name, uint32_t width, uint32_t height, uint32_t bands, const std::vector<uint8_t> &buffer) {
        return fpng::fpng_encode_image_to_file(file_name, (void *) buffer.data(), width, height, bands, 0);
    }

    bool read_png_file(const char *file_name, const uint32_t width, const uint32_t height, const uint32_t num_bands, std::vector<uint8_t> &buffer) {
        auto error =  fpng::fpng_decode_file(file_name, buffer, const_cast<uint32_t &>(width), const_cast<uint32_t &>(height), const_cast<uint32_t &>(num_bands), 4);
        if (error != fpng::FPNG_DECODE_SUCCESS) {
            std::cout << "Error " << std::to_string(error) << " reading png file: " << file_name << std::endl;
            return false;
        }
        return true;
    }


    bool write_tile(const std::vector<uint8_t> &img_data, size_t width, size_t height, size_t bands, const std::string &out_path) {
        // std::cout << "writing " << out_path << std::endl;
        return FASTTILER::PNG_IO::write_png_file(out_path.c_str(), width, height, bands, img_data);
    }
    bool write_tile(const std::vector<uint8_t> &img_data, size_t in_width, size_t in_height, size_t out_width, size_t out_height, size_t x_offset, size_t y_offset, size_t bands, const std::string &out_path) {
        std::__1::vector<uint8_t> offset_img(out_width * out_height * bands);

        const size_t initial_offset = ((y_offset * out_width) + x_offset) * bands;

        for (size_t y = 0; y < in_height; y++) {
            size_t in_img_start = (y * in_width * bands);
            size_t in_img_end = in_img_start + (in_width * bands);
            size_t out_img_start = initial_offset + (y * out_width * bands);
            std::copy(img_data.begin() + in_img_start, img_data.begin() + in_img_end, offset_img.begin() + out_img_start);
        }

        return FASTTILER::PNG_IO::write_png_file(out_path.c_str(), out_width, out_height, bands, offset_img);
    }
}// namespace PNG_IO