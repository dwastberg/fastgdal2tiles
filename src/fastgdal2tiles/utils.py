from pathlib import Path
from collections import defaultdict
import shutil

def create_out_dir_stucture(tile_details, job_info, out_dir, clean=False):
    if clean:
        shutil.rmtree(out_dir)
    for tile in tile_details:
        tile_path = Path(out_dir, str(tile.tz), str(tile.tx))
        tile_path.mkdir(parents=True, exist_ok=True)
    for tz in range(job_info.tmaxz - 1, job_info.tminz - 1, -1):
        tminx, tminy, tmaxx, tmaxy = job_info.tminmax[tz]
        for ty in range(tmaxy, tminy - 1, -1):
            for tx in range(tminx, tmaxx + 1):
                tile_path = Path(out_dir, str(tz), str(tx))
                tile_path.mkdir(parents=True, exist_ok=True)


def create_tile_pyramids(tile_details, job_info, out_dir):
    overview_tiles = defaultdict(list)
    pyramid_tiles = defaultdict(list)
    for tz in range(job_info.tmaxz - 1, job_info.tminz - 1, -1):
        tminx, tminy, tmaxx, tmaxy = job_info.tminmax[tz]
        for ty in range(tmaxy, tminy - 1, -1):
            for tx in range(tminx, tmaxx + 1):
                overview_tiles[tz].append((tx, ty))

                low_tile = (tz, tx, ty)
                for y in range(2 * ty, 2 * ty + 2):
                    for x in range(2 * tx, 2 * tx + 2):
                        minx, miny, maxx, maxy = job_info.tminmax[tz + 1]
                        if x >= minx and x <= maxx and y >= miny and y <= maxy:
                            if (ty == 0 and y == 1) or (
                                ty != 0 and (y % (2 * ty)) != 0
                            ):
                                tileposy = 0
                            else:
                                tileposy = job_info.tile_size
                            if tx:
                                tileposx = x % (2 * tx) * job_info.tile_size
                            elif tx == 0 and x == 1:
                                tileposx = job_info.tile_size
                            else:
                                tileposx = 0
                            pyramid_tiles[low_tile].append(
                                tz + 1, x, y, tileposx, tileposy
                            )

    return overview_tiles
