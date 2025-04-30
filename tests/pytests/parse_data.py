import sys
from pathlib import Path
from sodatools import read_path
import base64
import pickle

CURRENT = Path(__file__).resolve().parent

sys.path.insert(0, str(CURRENT))


if __name__ == "__main__":
    from test_unpack import RUNDATA

    data = CURRENT.parent / "data.txt"
    content = read_path(data)
    c2 = base64.b64decode(content)
    c3 = pickle.loads(c2)

    rundata = RUNDATA.from_buffer_copy(c3)
    print(rundata)
