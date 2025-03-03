#pragma once

#include <windows.h>
#include <winwlx.h>

extern "C" {
LONG CALLBACK CPlApplet(HWND hwnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
STDAPI DllCanUnloadNow(void);
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut);
VOID APIENTRY SuRunLogoffUser(PWLX_NOTIFICATION_INFO Info);
}