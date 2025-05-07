import subprocess

from sodatools import CD
from pathlib import Path

CR = Path(__file__).resolve().parent
PROJ_ROOT = CR.parent.joinpath("src/PC")
Script_Dir = CR.parent.joinpath("scripts")
script = Script_Dir.joinpath("build.ps1")


def build():
    with CD(PROJ_ROOT):
        try:
            subprocess.run(["pwsh", "-nop", script], check=False)
        except KeyboardInterrupt:
            return
