from pathlib import Path
import subprocess
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-32", "--mingw32", type=bool, action="store_true", help="32")
    parser.add_argument("-64", "--ucrt64", type=bool, action="store_true", help="64")