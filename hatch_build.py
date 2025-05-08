import contextlib
from typing import Any

from hatchling.builders.hooks.plugin.interface import BuildHookInterface


@contextlib.contextmanager
def CD(d: str):
    import os

    old = os.getcwd()
    os.chdir(d)
    yield
    os.chdir(old)


def build_wheel():
    pass

def build_sdist():
    import base64
    from pathlib import Path
    cr = Path(__file__).resolve().parent
    # Binary data
    binary_data = cr.joinpath("src/PC/Debug/InstallSuRun.exe").read_bytes()

    # Encode to Base64
    base64_encoded_data = base64.b64encode(binary_data)
    str2 = base64_encoded_data.decode("ascii")
    cr.joinpath("surun_tools/data.txt").write_text(str2)

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
