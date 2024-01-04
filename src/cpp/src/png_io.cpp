//
// Created by Dag Wästberg on 2024-01-03.
//
#include "png_io.h"

#include <fpng/fpng.h>
#include <cstdint>

using namespace FASTTILER;

bool PNG_IO::write_png_file(const char *file_name, uint32_t width, uint32_t height, uint32_t bands, std::vector<uint8_t> &buffer) {
    return fpng::fpng_encode_image_to_file(file_name, (void *)buffer.data(), width, height, bands, 0);

}

bool PNG_IO::read_png_file(const char *file_name, uint32_t  &width, uint32_t &height, uint32_t &num_bands, std::vector<uint8_t> &buffer) {
    return fpng::fpng_decode_file(file_name, buffer, width, height, num_bands, 4);
}




