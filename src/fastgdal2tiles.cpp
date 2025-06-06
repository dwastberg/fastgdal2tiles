//
// Created by Dag Wästberg on 2024-03-06.
//

#include <filesystem>
#include <iostream>
#include <tuple>
#include <cctype>
#include <chrono>
#include <iomanip>

#include "argh.h"


#include "TileInfo.h"
#include "fasttiler.h"
using namespace std;

std::string usage = "Usage: fastgdal2tiles -z <zoom levels> input_file output_dir\n\
  -z, --zoom <zoom levels>  Zoom levels to render. Can be a single number or a range, e.g. 5-10\n\
  -q, --quite               Do not output any info\n\
  -r, --resume              Resume rendering if tiles already exist\n";



bool is_number(const std::string &s) {
    if (s.empty())
        return false;
    for (char c: s)
        if (!isdigit(c))
            return false;
    return true;
}

pair<size_t,size_t> parse_zoom_levels(string zoom_arg) {

    if (!isdigit(zoom_arg[0])) {
        throw invalid_argument("Invalid zoom argument");
    }
    if (zoom_arg.find("-") != string::npos) {
        size_t dash_pos = zoom_arg.find("-");
        string min_zoom_str = zoom_arg.substr(0, dash_pos);
        string max_zoom_str = zoom_arg.substr(dash_pos + 1);
        if (!is_number(min_zoom_str) || !is_number(max_zoom_str)) {
            throw invalid_argument("Invalid zoom argument");
        }
        size_t min_zoom = stoi(zoom_arg.substr(0, dash_pos));
        size_t max_zoom = stoi(zoom_arg.substr(dash_pos + 1));
        if (min_zoom > max_zoom) {
            swap(min_zoom, max_zoom);
        }
        return make_pair(min_zoom, max_zoom);
    } else {
        size_t zoom = stoi(zoom_arg);
        return make_pair(zoom, zoom);
    }
}

size_t total_tiles(FASTTILER::TileInfo &tile_info) {
    size_t total = tile_info.td_vec.size();
    for (size_t z = tile_info.min_zoom; z <= tile_info.max_zoom-1; z++) {
        total += tile_info.tile_pyramid.at(z).size();
    }
    return total;
}


int main(int argc, char *argv[]) {
    argh::parser cmdl;
    cmdl.add_params({ "-z", "--zoom", "-h", "-r", "--resume", "--help" });
    cmdl.parse(argc, argv);


    if (cmdl.size() !=3) {
        cout << "Wrong number of arguments\n";
        cout << usage << std::endl;
        return 1;
    }

    if (cmdl[{"-h", "--help"}]) {
        cout << usage << std::endl;
        return 0;
    }

    int min_zoom = -1;
    int max_zoom = -1;
    string zoom_arg;
    bool quiet = cmdl[{ "-q", "--quite" }];
    bool resume = cmdl[{ "-r", "--resume" }];

    cmdl({"-z", "--zoom"}) >> zoom_arg;

    if (!zoom_arg.empty()) {
        try {
            auto zooms = parse_zoom_levels(zoom_arg);
            min_zoom = zooms.first;
            max_zoom = zooms.second;
        } catch (const std::invalid_argument &e) {
            cout << "Invalid zoom argument: " << zoom_arg << "\n";
            cout << usage << std::endl;
            return 1;
        }
    }

    filesystem::path input_file = cmdl[1];
    filesystem::path output_dir = cmdl[2];
    if (!filesystem::exists(input_file)) {
        cout << "Input file does not exist: " << input_file << "\n";
        return 1;
    }

    auto start_time = chrono::high_resolution_clock::now();
    auto tile_info = FASTTILER::TileInfo(input_file, min_zoom, max_zoom);
//    auto tzminmax = tile_info.build_tzminmax();
//    auto base_tiles = tile_info.build_tile_details();

    if (!quiet) {
        auto total = total_tiles(tile_info);
        std::cout << "Rendering "<< total << " tiles from zoom level " << tile_info.min_zoom << " to " << tile_info.max_zoom << std::endl;
    }
    FASTTILER::render_tiles(input_file, tile_info, output_dir, resume, !quiet, 0.2);
    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    auto elapsed_seconds = elapsed / std::chrono::seconds(1);
    if (!quiet)
        std::cout << "Tiles rendered in: " << std::fixed << std::setprecision(2) << elapsed_seconds << " s" << std::endl;
    return 0;

}
