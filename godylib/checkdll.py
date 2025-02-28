import struct
import os


def check_dll_architecture(dll_path):
    """
    Checks if a DLL is 32-bit or 64-bit.

    Args:
        dll_path: The path to the DLL file.

    Returns:
        "32-bit" if the DLL is 32-bit, "64-bit" if it's 64-bit,
        or None if an error occurs or the DLL is invalid.
    """
    try:
        with open(dll_path, "rb") as f:
            # Read the DOS header
            dos_header = f.read(64)

            # Check for the PE magic number
            if dos_header[:2] != b"MZ":
                return None

            # Get the PE header offset
            pe_offset = struct.unpack("<L", dos_header[60:64])[0]

            # Seek to the PE header
            f.seek(pe_offset, os.SEEK_SET)

            # Read the PE header
            pe_header = f.read(4)

            # Check for the PE magic number
            if pe_header != b"PE\x00\x00":
                return None

            # Read the file header
            f.seek(pe_offset + 4, os.SEEK_SET)
            file_header = f.read(20)

            # Get machine type
            machine_type = struct.unpack("<H", file_header[:2])[0]

            # Check the machine type to determine architecture
            if machine_type == 0x014C:
                return "32-bit"
            elif machine_type == 0x8664:
                return "64-bit"
            else:
                return None
    except Exception:
        return None


# Example usage:
dll_path = "lib.dll"  # Replace with the actual path to your DLL
architecture = check_dll_architecture(dll_path)

if architecture:
    print(f"The DLL is {architecture}")
else:
    print("Could not determine DLL architecture or an error occurred.")
