import glob

from sodatools import CD, read_path, write_path  # noqa: F401
from pathlib import Path

files = list(glob.glob("*.h"))
files.extend(list(glob.glob("*.cpp")))

# files.extend(list(glob.glob("SuRunC/*.h")))
# files.extend(list(glob.glob("SuRunC/*.cpp")))

# files.extend(list(glob.glob("SuRunExt/*.h")))
# files.extend(list(glob.glob("SuRunExt/*.cpp")))

CURRENT = Path(__file__).resolve().parent
for file in files:
    file = Path(file)
    if file.name == "main.h":
        continue

    c = ""
    try:
        c = read_path(file)
    except UnicodeDecodeError:
        print(file)
        continue
    lines = c.split("\n")
    if "clang-format" not in lines[0]:
        prepend = ["// clang-format off", "//go:build ignore", "// clang-format on"]
        prepend.extend(lines)
        write_path(file, "\n".join(prepend))
