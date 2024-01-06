//
// Created by Dag Wästberg on 2023-01-11.
//

#ifndef FASTTILER_PNG_IO_H
#define FASTTILER_PNG_IO_H

#include <cstdint>
#include <vector>
#include "fpng/fpng.h"


namespace FASTTILER::PNG_IO {

    bool write_png_file(const char *file_name, uint32_t width, uint32_t height, uint32_t  bands, const std::vector<uint8_t> &buffer);

    bool read_png_file(const char *file_name, uint32_t  &width, uint32_t &height, uint32_t  & num_bands, std::vector<uint8_t> &buffer);

} // FASTTILER::PNG_IO
#endif //FASTTILER_PNG_IO_H
