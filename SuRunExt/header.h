#pragma once

#include <windows.h>
#include <winwlx.h>

extern "C" {
#define DllExport __declspec(dllexport)

LONG CALLBACK DllExport CPlApplet(HWND hwnd, UINT uMsg, LPARAM lParam1,
                                  LPARAM lParam2);
STDAPI DllExport DllCanUnloadNow(void);
STDAPI DllExport DllGetClassObject(REFCLSID rclsid, REFIID riid,
                                   LPVOID *ppvOut);
VOID APIENTRY DllExport SuRunLogoffUser(PWLX_NOTIFICATION_INFO Info);
}