import win32file
import pywintypes
import win32pipe

pipe_name = r'\\.\pipe\SuRunDebug'

pipe_handle = win32pipe.CreateNamedPipe(
    pipe_name,
    win32pipe.PIPE_ACCESS_DUPLEX,
    win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_READMODE_MESSAGE | win32pipe.PIPE_WAIT,
    1,  # Max instances
    65536,  # Out buffer size
    65536,  # In buffer size
    0,  # Default timeout
    None
)
print(f"Named pipe '{pipe_name}' created successfully.")
try:
    handle = win32file.CreateFile(
        pipe_name,
        win32file.GENERIC_WRITE | win32file.GENERIC_READ, 0, None,
        win32file.OPEN_EXISTING, 0, None)
    print("Connected to pipe")
except pywintypes.error as e:
    print(f"Error connecting to pipe: {e}")
    exit()

try:
    data_to_send = "Hello from Python!".encode('utf-8')
    win32file.WriteFile(handle, data_to_send)
    print("Data sent successfully")
except pywintypes.error as e:
        print(f"Error writing to pipe: {e}")

win32file.CloseHandle(handle)


pipe_handle = win32file.CreateFile(
        pipe_name,
        win32file.GENERIC_READ,
        0,  # no sharing
        None,
        win32file.OPEN_EXISTING,
        0,  # default attributes
        None
    )

try:
    while True:
        hr, data = win32file.ReadFile(handle, 18500) # Read in 64kb chunks
        if hr == 0: # Check for successful read
            if data:
                print(f"Received: {data.decode()}")
            else:
                print("Pipe closed by client.")
                break
        else:
            print(f"Error reading from pipe: {hr}")
            break
except pywintypes.error as e:
    print(f"Error during read operation: {e}")