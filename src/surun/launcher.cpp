// clang-format off
//go:build ignore
// clang-format on

#include "main.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(hPrevInst);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  _WinMain(NULL, NULL, GetCommandLine(), 0);
}