from sodatools import CD
import glob
from pathlib import Path

files = list(glob.glob("*.h"))
files.extend(list(glob.glob("*.cpp")))

files.extend(list(glob.glob("SuRunC/*.h")))
files.extend(list(glob.glob("SuRunC/*.cpp")))

files.extend(list(glob.glob("SuRunExt/*.h")))
files.extend(list(glob.glob("SuRunExt/*.cpp")))

CURRENT = Path(__file__).resolve().parent
for file in files:
    with CD(CURRENT.parent):
        import subprocess

        subprocess.run(
            [
                "C:/src/llvm-project-install/bin/clang-format",
                CURRENT.name + "/" + file,
                "-i",
            ]
        )
