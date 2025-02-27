import win32file
import pywintypes
import win32pipe
import win32event
import winerror
# https://resources.oreilly.com/examples/9781565926219/-/tree/master/ch18_services

pipeName = r"\\.\pipe\SuRunDebug"


openMode = win32pipe.PIPE_ACCESS_DUPLEX | win32file.FILE_FLAG_OVERLAPPED
pipeMode = win32pipe.PIPE_TYPE_MESSAGE

# When running as a service, we must use special security for the pipe
sa = pywintypes.SECURITY_ATTRIBUTES()
# Say we do have a DACL, and it is empty
# (ie, allow full access!)
sa.SetSecurityDescriptorDacl(1, None, 0)

pipeHandle = win32pipe.CreateNamedPipe(
    pipeName,
    openMode,
    pipeMode,
    win32pipe.PIPE_UNLIMITED_INSTANCES,
    0,
    0,
    6000,  # default buffers, and 6 second timeout.
    sa,
)

hWaitStop = win32event.CreateEvent(None, 0, 0, None)
# We need to use overlapped IO for this, so we dont block when
# waiting for a client to connect.  This is the only effective way
# to handle either a client connection, or a service stop request.
overlapped = pywintypes.OVERLAPPED()
# And create an event to be used in the OVERLAPPED object.
overlapped.hEvent = win32event.CreateEvent(None, 0, 0, None)

# Loop accepting and processing connections
while 1:
    try:
        hr = win32pipe.ConnectNamedPipe(pipeHandle, overlapped)
    except Exception as e:
        print("Error connecting pipe!", e)
        pipeHandle.Close()
        break

    if hr == winerror.ERROR_PIPE_CONNECTED:
        # Client is fast, and already connected - signal event
        win32event.SetEvent(overlapped.hEvent)
    # Wait for either a connection, or a service stop request.
    timeout = win32event.INFINITE
    waitHandles = hWaitStop, overlapped.hEvent
    rc = win32event.WaitForMultipleObjects(waitHandles, 0, timeout)
    if rc == win32event.WAIT_OBJECT_0:
        # Stop event
        break
    else:
        # Pipe event - read the data, and write it back.
        # (We only handle a max of 255 characters for this sample)
        try:
            hr, data = win32file.ReadFile(pipeHandle, 18500)
            # win32file.WriteFile(pipeHandle, ("You sent me:" + data.decode()).encode())
            import pickle

            d = pickle.dumps(data)
            import base64

            d_s = base64.b64encode(d)
            from pathlib import Path

            Path("data.txt").write_bytes(d_s)
            # And disconnect from the client.
            win32pipe.DisconnectNamedPipe(pipeHandle)
        except win32file.error:
            # Client disconnected without sending data
            # or before reading the response.
            # Thats OK - just get the next connection
            continue
