#include "main.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(hPrevInst);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  _WinMain(NULL, NULL, GetCommandLine(), 0);
}