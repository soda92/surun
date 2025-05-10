from pathlib import Path
import os

cr = Path(__file__).resolve().parent


def main():
    os.startfile(str(cr.joinpath("install.exe")))


if __name__ == "__main__":
    main()
