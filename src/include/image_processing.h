//
// Created by Dag Wästberg on 2023-11-29.
//

#ifndef FASTGDAL2TILES_IMAGE_PROCESSING_H
#define FASTGDAL2TILES_IMAGE_PROCESSING_H

#include <vector>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize2.h"

namespace FASTTILER::IMAGE_PROCESSING {
    bool shrink_tile(const std::vector<uint8_t> &tile_data, const size_t in_size, const size_t bands, const size_t out_size, std::vector<uint8_t> &output_data) {
        if (in_size == out_size) {
            output_data = tile_data;
            return true;
        }

        auto resize_result = stbir_resize_uint8_srgb(tile_data.data(), in_size, in_size, 0, output_data.data(), out_size, out_size, 0, STBIR_RGBA_PM);
        if (resize_result == 0)
            return false;
        return true;
    }


} // FASTTILER::IMAGE_PROCESSING

#endif //FASTGDAL2TILES_IMAGE_PROCESSING_H
