from sodatools import CD
from pathlib import Path

CR = Path(__file__).resolve().parent
PROJ_ROOT = CR.parent.joinpath("src/PC")


def clean():
    with CD(PROJ_ROOT):
        dirs = (
            "DebugU",
            "DebugUsr32",
            "DebugUx64",
            "ReleaseU",
            "ReleaseUsr32",
            "ReleaseUx64",
            "x64",
            "x64 Unicode Debug",
            "Debug",
            "Release",
            "SuRun",
            "InstallSuRun",
            "SuRun32 Unicode Debug",
        )
        for i in dirs:
            if not Path(i).exists():
                continue
            try:
                import shutil

                shutil.rmtree(i)
            except Exception as e:
                print(e)
