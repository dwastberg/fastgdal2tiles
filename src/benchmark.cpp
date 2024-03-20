//
// Created by Dag Wästberg on 2024-03-20.
//

#include <filesystem>
#include <iostream>
#include <tuple>
#include <cctype>
#include <chrono>
#include <iomanip>

#include "TileInfo.h"
#include "fasttiler.h"
using namespace std;

int main()
{
    size_t min_zoom = 19;
    size_t max_zoom = 22;
    filesystem::path input_file = "data/fullsize.tif";
    auto start_time = chrono::high_resolution_clock::now();
    filesystem::path output_dir = "data/tmp_output";
    auto tile_info = FASTTILER::TileInfo(input_file, min_zoom, max_zoom);
    //    auto tzminmax = tile_info.build_tzminmax();
    //    auto base_tiles = tile_info.build_tile_details();

    size_t num_runs =3;
   for (size_t i = 1; i<10; i++) {
       float thread_ratio = 0.1 * i;
       double elapsed_seconds = 0;
       for (size_t r = 0; r<num_runs; r++) {
           auto start_time = chrono::high_resolution_clock::now();
           FASTTILER::render_tiles(input_file, tile_info, output_dir, false, thread_ratio);
           std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
           elapsed_seconds += elapsed / std::chrono::seconds(1);
       }
         elapsed_seconds /= num_runs;
         cout << "thread_ratio = " << thread_ratio << " elapsed_seconds = " << elapsed_seconds << "\n";
   }

}