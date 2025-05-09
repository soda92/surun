from pathlib import Path
from sodatools import write_path
import sys
import json
import argparse

CURRENT = Path(__file__).resolve().parent


def get_file(s: str):
    return CURRENT.joinpath(s)


sys.path.insert(0, CURRENT)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--builddir", "-B", type=str, default="build", help="build dir")
    parser.add_argument(
        "--installer-builddir", "-I", type=str, default="build-i", help="build dir"
    )
    args = parser.parse_args()
    from gen_lsp_tdm import fix_commands
    if not Path(f"{args.installer_builddir}/compile_commands.json").exists():
        exit(0)

    c2 = fix_commands(get_file(f"{args.installer_builddir}/compile_commands.json"))
    c1 = fix_commands(get_file(f"{args.builddir}/compile_commands.json"))

    c1.extend(c2)
    s = json.dumps(c1, indent=2)
    write_path(CURRENT.joinpath("compile_commands.json"), s)
