from pathlib import Path


def create_out_dir_stucture(tile_details, job_info, out_dir):
    for tile in tile_details:
        tile_path = Path(out_dir, str(tile.tz), str(tile.tx))
        tile_path.mkdir(parents=True, exist_ok=True)
    for tz in range(job_info.tmaxz - 1, job_info.tminz - 1, -1):
        tminx, tminy, tmaxx, tmaxy = job_info.tminmax[tz]
        for ty in range(tmaxy, tminy - 1, -1):
            for tx in range(tminx, tmaxx + 1):
                tile_path = Path(out_dir, str(tz), str(tx), str(ty))
                tile_path.mkdir(parents=True, exist_ok=True)
