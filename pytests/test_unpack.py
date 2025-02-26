import ctypes
import platform

# Define constants based on the C header file.
# You might need to adjust these based on your specific environment.
# 'UNLEN' and 'MAX_PATH' would ideally come from a header file,
# but since we don't have that, we're making reasonable guesses
# based on common Windows values.

# Check architecture
is_64bit = platform.architecture()[0] == "64bit"

# Guess the sizes
UNLEN = 256  # Typical max username length + 1
MAX_PATH = 260  # Typical windows value


class RUNDATA(ctypes.Structure):
    # IMPORTANT: Use _pack_ = 1 to match #pragma pack(1)
    _pack_ = 1
    _fields_ = [
        ("CliProcessId", ctypes.c_uint32),  # DWORD
        ("CliThreadId", ctypes.c_uint32),  # DWORD
        ("SessionID", ctypes.c_uint32),  # DWORD
        ("WinSta", ctypes.c_wchar * MAX_PATH),  # WCHAR[MAX_PATH]
        ("Desk", ctypes.c_wchar * MAX_PATH),  # WCHAR[MAX_PATH]
        ("UserName", ctypes.c_wchar * (UNLEN + UNLEN + 2)),  # WCHAR[UNLEN + UNLEN + 2]
        ("cmdLine", ctypes.c_wchar * 4096),  # WCHAR[4096]
        ("CurDir", ctypes.c_wchar * 4096),  # WCHAR[4096]
        ("KillPID", ctypes.c_uint32),  # DWORD
        ("RetPID", ctypes.c_uint32),  # DWORD
        (
            "RetPtr",
            ctypes.c_uint64 if is_64bit else ctypes.c_uint32,
        ),  # unsigned __int64
        ("NewPID", ctypes.c_uint32),  # DWORD
        ("IconId", ctypes.c_int),  # int
        ("TimeOut", ctypes.c_uint32),  # DWORD
        ("bShlExHook", ctypes.c_bool),  # bool
        ("beQuiet", ctypes.c_bool),  # bool
        ("bRunAs", ctypes.c_ubyte),  # BYTE
        ("Groups", ctypes.c_uint32),  # DWORD
        ("bShExNoSafeDesk", ctypes.c_bool),  # bool
        # ("ConsolePID", ctypes.c_uint32),    # DWORD  Commented out, as in the original
    ]


# Example usage (assuming you have a byte buffer 'data' containing the struct data)
def test_rundata(data):
    try:
        rundata = RUNDATA.from_buffer_copy(data)

        print(f"CliProcessId: {rundata.CliProcessId}")
        print(f"CliThreadId: {rundata.CliThreadId}")
        print(f"SessionID: {rundata.SessionID}")
        print(f"WinSta: {rundata.WinSta}")
        print(f"Desk: {rundata.Desk}")
        print(f"UserName: {rundata.UserName}")
        print(f"cmdLine: {rundata.cmdLine}")
        print(f"CurDir: {rundata.CurDir}")
        print(f"KillPID: {rundata.KillPID}")
        print(f"RetPID: {rundata.RetPID}")
        print(f"RetPtr: {rundata.RetPtr}")
        print(f"NewPID: {rundata.NewPID}")
        print(f"IconId: {rundata.IconId}")
        print(f"TimeOut: {rundata.TimeOut}")
        print(f"bShlExHook: {rundata.bShlExHook}")
        print(f"beQuiet: {rundata.beQuiet}")
        print(f"bRunAs: {rundata.bRunAs}")
        print(f"Groups: {rundata.Groups}")
        print(f"bShExNoSafeDesk: {rundata.bShExNoSafeDesk}")
        # print(f"ConsolePID: {rundata.ConsolePID}") # Commented out

        # Example of packing back into bytes:
        packed_data = bytes(rundata)
        assert packed_data == data, "Packing failed!"  # Sanity check
        print("Packing/Unpacking successful!")
    except ValueError as e:
        print(f"ERROR: Could not parse struct: {e}")
        print(f"Size of the buffer expected {ctypes.sizeof(RUNDATA)}")
        print(f"Size of the data given = {len(data)}")


# --- Test ---

# 1. Create sample data (as if it came from the C struct)
# It is crucial to create data *exactly* matching the packed struct, including endianness.
# We are assuming little-endian here (typical for Windows).

sample_data = (
    b"\x01\x00\x00\x00"  # CliProcessId (1)
    b"\x02\x00\x00\x00"  # CliThreadId (2)
    b"\x03\x00\x00\x00",  # SessionID (3)
    b"WinSta0\0".ljust(MAX_PATH * 2, b"\0"),  # WinSta, padded with nulls
    b"Default\0".ljust(MAX_PATH * 2, b"\0"),  # Desk, padded
    b"TestUser\0".ljust((UNLEN + UNLEN + 2) * 2, b"\0"),  # UserName
    b"cmd.exe /c dir\0".ljust(4096 * 2, b"\0"),  # cmdLine
    b"C:\\Windows\\System32\0".ljust(4096 * 2, b"\0"),  # CurDir
    b"\x04\x00\x00\x00",  # KillPID (4)
    b"\x05\x00\x00\x00"  # RetPID (5)
    + (
        b"\x06\x00\x00\x00\x00\x00\x00\x00" if is_64bit else b"\x06\x00\x00\x00"
    ),  # RetPtr (6)
    b"\x07\x00\x00\x00"  # NewPID (7)
    b"\x08\x00\x00\x00"  # IconId (8)
    b"\x09\x00\x00\x00"  # TimeOut (9)
    b"\x01"  # bShlExHook (True)
    b"\x00"  # beQuiet (False)
    b"\x03"  # bRunAs (bit flags)
    b"\x0a\x00\x00\x00"  # Groups (10)
    b"\x01"  # bShExNoSafeDesk (True)
    # b'\x0b\x00\x00\x00'          # ConsolePID (11) , commented out
)

x = b""
for i in sample_data:
    x += i

# 2. Test the struct
test_rundata(x)


# Example of creating and packing an instance:
rundata_instance = RUNDATA()
rundata_instance.CliProcessId = 1234
rundata_instance.WinSta = "WinSta0"
rundata_instance.UserName = "MyUser"
rundata_instance.bRunAs = 0b00000101  # Example bit flags

packed_instance = bytes(rundata_instance)

# Test the packed instance
test_rundata(packed_instance)
