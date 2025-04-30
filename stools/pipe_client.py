import win32pipe

if __name__ == "__main__":
    message = "aaa"
    pipeName = r"\\.\pipe\SuRunDebug"
    data = win32pipe.CallNamedPipe(pipeName, message.encode(), 512, 0)
    print("The service sent back:")
    print(data)
