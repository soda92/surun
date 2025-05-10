from pathlib import Path

import argparse
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("version", help="target version")
args = parser.parse_args()
version: str = args.version

tree = subprocess.getoutput("git status")
if "working tree clean" not in tree:
    print("please commit or stash changes first")
    exit(-1)

if not re.match(r"[0-9]{4}.[0-9]{1,2}.[0-9]{1,2}.[0-9]{1,2}", version):
    print(f"invalid version {version}")
    exit(-1)

src = Path(__file__).resolve().parent.joinpath("src/surun/SuRunVer.h")

content = src.read_text(encoding="utf8")

ver = version.split(".")
content = re.sub(r"#define VERSION_HH [0-9]+", f"#define VERSION_HH {ver[0]}", content)
content = re.sub(r"#define VERSION_HL [0-9]+", f"#define VERSION_HL {ver[1]}", content)
content = re.sub(r"#define VERSION_LH [0-9]+", f"#define VERSION_LH {ver[2]}", content)
content = re.sub(r"#define VERSION_LL [0-9]+", f"#define VERSION_LL {ver[3]}", content)
src.write_text(content, encoding="utf8")

toml = Path(__file__).resolve().parent.joinpath("pyproject.toml")
content = toml.read_text(encoding="utf8")
content = re.sub(
    'version = "[0-9]{4}.[0-9]{1,2}.[0-9]{1,2}.[0-9]{1,2}"',
    f'version = "{version}"',
    content,
)
toml.write_text(content, encoding="utf8")

subprocess.run("git add .", shell=True, check=True)
subprocess.run(["git", "commit", "-m", f"update version to {version}"], check=True)
subprocess.run(["git", "tag", version], check=True)
subprocess.run(["git", "push"], check=True)
subprocess.run(["git", "push", "--tags"], check=True)
