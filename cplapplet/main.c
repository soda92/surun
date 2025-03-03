#include <windows.h>
#include <cpl.h>
#include "resource.h"

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved) {
  return TRUE;
}

LONG CALLBACK CPlApplet(HWND hwnd, UINT uMsg, LONG lParam1, LONG lParam2) {
  LPCPLINFO lpCPlInfo;
  switch (uMsg) {
  case CPL_INIT: // first message, sent once
    // MessageBox(hwnd, TEXT("Hello, world!"), TEXT("CPL"), MB_OK);
    return TRUE;

  case CPL_GETCOUNT: // second message, sent once
    return 1L;       // (LONG)NUM_APPLETS;

  case CPL_INQUIRE: // third message, sent once per app
    lpCPlInfo = (LPCPLINFO)lParam2;

    lpCPlInfo->idIcon = IDI_SAMPLE_CPL;
    lpCPlInfo->idName = IDS_SAMPLE_CPL_NAME;
    lpCPlInfo->idInfo = IDS_SAMPLE_CPL_DESCRIPTION;
    lpCPlInfo->lData = 0L;
    break;

  case CPL_DBLCLK: // application icon double-clicked
  {
    MessageBox(hwnd, TEXT("Hello, world!"), TEXT("CPL"), MB_OK);
  } break;
  }
  return 0;
}