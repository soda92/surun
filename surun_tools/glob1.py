import glob
from pathlib import Path
from sodatools import str_path

proj_root = Path(__file__).resolve().parent.parent


def glob_dirs(*args):
    ret = []
    for d in args:
        d = list(glob.glob(f"**/{d}", root_dir=proj_root, recursive=True))
        d = list(map(proj_root.joinpath, d))
        d = list(filter(Path.is_dir, d))
        ret.extend(d)
    return ret

def glob_files(*args):
    ret = []
    for d in args:
        d = list(glob.glob(f"**/{d}", root_dir=proj_root, recursive=True))
        d = list(map(proj_root.joinpath, d))
        d = list(filter(Path.is_file, d))
        ret.extend(d)
    return ret


def rm_dirs(*args):
    import shutil

    for d in args:
        if isinstance(d, list):
            for dir in d:
                dir = str_path(dir)
                try:
                    shutil.rmtree(dir)
                except Exception as e:
                    print(e)
                    continue
        elif isinstance(d, str):
            shutil.rmtree(d)
