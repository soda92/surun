from sodatools import CD
from pathlib import Path
from surun_tools.glob1 import glob_files
import subprocess

proj_root = Path(__file__).resolve().parent.parent

files = []

def surun_format():
    with CD(proj_root):
        files.extend(glob_files("*.h"))
        files.extend(glob_files("*.cpp"))

        files.extend(glob_files("SuRunC/*.h"))
        files.extend(glob_files("SuRunC/*.cpp"))

        files.extend(glob_files("SuRunExt/*.h"))
        files.extend(glob_files("SuRunExt/*.cpp"))

    for file in files:
        file_path = proj_root.joinpath(file)
        subprocess.run(
            [
                "clang-format",
                file_path,
                "-i",
            ]
        )
