import contextlib
import subprocess
from typing import Any

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from pathlib import Path


@contextlib.contextmanager
def CD(d: str):
    import os

    old = os.getcwd()
    os.chdir(d)
    yield
    os.chdir(old)


CR = Path(__file__).resolve().parent
PROJ_ROOT = CR
Script_Dir = PROJ_ROOT.joinpath("scripts")
script = Script_Dir.joinpath("build.ps1")


def build():
    with CD(PROJ_ROOT):
        try:
            subprocess.run(["pwsh", "-nop", script], check=False)
        except KeyboardInterrupt:
            return


def build_wheel():
    build()
    import shutil

    shutil.copy(
        PROJ_ROOT.joinpath("src/PC/Debug/InstallSuRun.exe"),
        PROJ_ROOT.joinpath("surun_tools/install.exe"),
    )


def build_sdist():
    pass


class CustomBuilder(BuildHookInterface):
    def initialize(
        self,
        version: str,  # noqa: ARG002
        build_data: dict[str, Any],
    ) -> None:
        build_data["tag"] = "py3-none-win_amd64"
        if self.target_name == "sdist":
            build_sdist()
        else:
            build_wheel()


if __name__ == "__main__":
    build_wheel()
