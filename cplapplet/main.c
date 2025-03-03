#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved) {
  return TRUE;
}

LONG CALLBACK CPlApplet(HWND hwnd, UINT uMsg, LONG lParam1, LONG lParam2) {
  MessageBox(hwnd, TEXT("Hello, world!"), TEXT("CPL"), MB_OK);
  return 0;
}