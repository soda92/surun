---
date: '2025-05-10T10:58:39+08:00'
title: 'Formatting Cpp Files'
---

Actually, formatting C++ files are hard.

VSCode comes with a default formatting style - backed by MS C/C++ or clang-format. Also, most of legacy code
are not formatted using standard tools, so you will get a lot of git differences when you have changed a
bit of code, and pressed Alt+Shift+F; this is a key difference compared to Python.

The solution was to format all C++ files before touching the code - so an automated tool was necessary.

However, after the first round, it will be time-consuming to format all files, even if you just changed
code in a few files. In this case, we can extract changed files from git, and only format these files.

One of the key configuration for clang-format is the `SortIncludes: Never` option. On Windows, you will always
need "windows.h" before any other things, and the default formatting style will put angled includes in the second
place, render a major build error.

refer [`run_format.py`][1] for a git formatting implementation; code also provided below.

[1]: https://github.com/soda92/surun/blob/main/surun_tools/run_format.py


```python
from sodatools import CD
from pathlib import Path
from surun_tools.glob1 import glob_files
import subprocess

proj_root = Path(__file__).resolve().parent.parent

files = []


def surun_format_all():
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


def surun_format():
    files = []
    with CD(proj_root):
        changed_files = subprocess.getoutput("git status --short")
        for line in changed_files.split("\n"):
            if line.startswith(" M "):
                files.append(line[3:])

    for file in files:
        file_path = proj_root.joinpath(file)
        subprocess.run(
            [
                "clang-format",
                file_path,
                "-i",
            ]
        )

```
