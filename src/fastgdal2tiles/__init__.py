import gdal2tiles
from pathlib import Path
from fastgdal2tiles.utils import create_out_dir_stucture
from fastgdal2tiles import _fastgdal2tiles
from time import time


def main(
    in_file: Path, out_dir: Path, min_zoom: int, max_zoom: int, tms: bool, resume: bool
) -> bool:
    print("Starting Tiling")
    options = {}
    if min_zoom is not None:
        options["zoom"] = (min_zoom, max_zoom)
    options["resume"] = resume
    options["resampling"] = "near"
    options["tmscompatible"] = tms

    g2t_options = gdal2tiles.process_options(str(in_file), str(out_dir), options)
    job_info, tile_details = gdal2tiles.worker_tile_details(
        str(in_file), str(out_dir), g2t_options
    )

    create_out_dir_stucture(tile_details, job_info, out_dir)

    min_zoom = job_info.tminz
    max_zoom = job_info.tmaxz

    start_time = time()
    _fastgdal2tiles.render_tiles(
        job_info.src_file, str(out_dir), min_zoom, max_zoom, tile_details
    )
    print(f"rendering base tiles: {time()-start_time}")

    return True


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Converts a GDAL supported raster file to a set of tiles."
    )
    parser.add_argument("input", help="Input file to convert")
    parser.add_argument("output", help="Output directory for the generated tiles")
    parser.add_argument(
        "-z",
        "--zoom",
        help='Zoom levels to render (format: "2-5" or "10")',
        default=None,
    )
    parser.add_argument(
        "-p",
        "--profile",
        help="Select the output profile (mercator,geodetic,raster)",
        default="mercator",
    )
    parser.add_argument("-e", "--resume", help="Resume mode", action="store_true")
    parser.add_argument(
        "-r",
        "--resampling",
        help="Resampling method (average, near, bilinear, cubic, cubicspline, lanczos, mode)",
        default="average",
    )
    parser.add_argument(
        "-t",
        "--tms",
        help="Create TMS Global Mercator Profile tiles",
        action="store_true",
    )
    parser.add_argument(
        "-a",
        "--srcnodata",
        help="NODATA transparency value to assign to the input data",
    )
    parser.add_argument(
        "-n", "--dstnodata", help="NODATA transparency value to assign to the tiles"
    )
    parser.add_argument(
        "-w",
        "--webviewer",
        help="Generate a simple HTML viewer for the generated tiles",
        action="store_true",
    )
    parser.add_argument("-v", "--verbose", help="Verbose mode", action="store_true")
    parser.add_argument(
        "-s",
        "--s_srs",
        help="The spatial reference system used for the source input data",
    )
    parser.add_argument(
        "-d", "--t_srs", help="The spatial reference system to use for the tiles"
    )
    args = parser.parse_args()

    in_file = Path(args.input)
    if not in_file.exists():
        print(f"Input file {in_file} does not exist")
        exit(1)
    out_dir = Path(args.output)
    zoom = args.zoom
    if zoom is not None:
        if "-" in zoom:
            min_zoom, max_zoom = zoom.split("-")
        else:
            min_zoom = max_zoom = zoom
        try:
            min_zoom = int(min_zoom)
            max_zoom = int(max_zoom)
        except ValueError:
            print("Zoom levels must be integers")
            exit(1)
        if min_zoom > max_zoom:
            print("Min zoom must be less than max zoom")
            exit(1)
    else:
        min_zoom = max_zoom = None

    main(in_file, out_dir, min_zoom, max_zoom, args.tms, args.resume)
