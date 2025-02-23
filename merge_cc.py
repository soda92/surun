from pathlib import Path
from sodatools import write_path
import sys
import json
import argparse

CURRENT = Path(__file__).resolve().parent

sys.path.insert(0, CURRENT)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--builddir", "-B", type=str, default="build", help="build dir")
    args = parser.parse_args()
    from gen_lsp_tdm import fix_commands

    c2 = fix_commands(Path("build-i/compile_commands.json"))
    c1 = fix_commands(Path(f"{args.builddir}/compile_commands.json"))

    c1.extend(c2)
    s = json.dumps(c1, indent=2)
    write_path(CURRENT.joinpath("compile_commands.json"), s)
