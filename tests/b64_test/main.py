import base64
from pathlib import Path
cr = Path(__file__).resolve().parent
# Binary data
binary_data = cr.parent.parent.joinpath("PC/InstallSuRun.exe").read_bytes()

# Encode to Base64
base64_encoded_data = base64.b64encode(binary_data)
str2 = base64_encoded_data.decode("ascii")
print(f"Base64 Encoded: {str2[:100]}")

# Decode from Base64
decoded_binary_data = base64.b64decode(str2)
print(f"Decoded Binary: {decoded_binary_data[:100]}")