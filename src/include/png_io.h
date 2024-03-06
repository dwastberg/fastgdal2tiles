//
// Created by Dag Wästberg on 2023-01-11.
//

#ifndef FASTTILER_PNG_IO_H
#define FASTTILER_PNG_IO_H

#include <cstdint>
#include <vector>
#include <string>
#include "fpng/fpng.h"


namespace FASTTILER::PNG_IO {

    bool write_png_file(const char *file_name, uint32_t width, uint32_t height, uint32_t  bands, const std::vector<uint8_t> &buffer);
    bool write_tile(const std::vector<uint8_t> &img_data, size_t width, size_t height, size_t bands, const std::string &out_path);
    bool write_tile(const std::vector<uint8_t> &img_data, size_t in_width, size_t in_height, size_t out_width, size_t out_height, size_t x_offset, size_t y_offset, size_t bands, const std::string &out_path);

    bool read_png_file(const char *file_name, const uint32_t width, const uint32_t height, const uint32_t num_bands, std::vector<uint8_t> &buffer);

} // FASTTILER::PNG_IO
#endif //FASTTILER_PNG_IO_H
