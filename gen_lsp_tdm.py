from pathlib import Path
import json
import os
import functools
import argparse
from sodatools import read_path, write_path, str_path

CURRENT = Path(__file__).resolve().parent


def which(name):
    path = os.environ["PATH"]
    for p in path.split(";"):
        exe = Path(p).joinpath(name)
        if exe.exists():
            return exe


def str_(s):
    return s.replace("\\", "/")


@functools.cache
def get_gcc_path():
    # out = which("gcc.exe")
    # return out.resolve().parent.parent
    return str_(r"C:\TDM-GCC-64\virtual")


def get_includes():
    ret = []
    includes = [
        r"{gcc}lib\gcc\x86_64-w64-mingw32\10.3.0\include\c++",
        r"{gcc}x86_64-w64-mingw32\include",
        r"{gcc}lib\gcc\x86_64-w64-mingw32\10.3.0\include\c++\x86_64-w64-mingw32",
    ]
    for i in includes:
        i = str_(i)
        i = i.replace("{gcc}", str_path(get_gcc_path()) + "/")
        ret.append("-isystem " + i)
    return ret


def fix_command(c) -> str:
    list_ = c.replace("\\", "/").split()
    list_.extend(get_includes())

    return " ".join(list_)


def fix_commands(c: Path):
    c = read_path(c)
    obj = json.loads(c)
    for item in obj:
        item["command"] = fix_command(item["command"])
    return obj


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--builddir", "-B", type=str, default="build", help="build dir")
    args = parser.parse_args()
    commands = CURRENT.joinpath(args.builddir).joinpath("compile_commands.json")
    obj = fix_commands(commands)
    s = json.dumps(obj, indent=2)
    write_path(CURRENT.joinpath("compile_commands.json"), s)
