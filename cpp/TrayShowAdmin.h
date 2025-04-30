// clang-format off
//go:build ignore
// clang-format on
#pragma once

#include <windows.h>
DWORD WINAPI TSAThreadProc(void *p);

void InitTrayShowAdmin();
BOOL ProcessTrayShowAdmin(BOOL bShowTray, BOOL bBalloon);
void CloseTrayShowAdmin();
BOOL StartTSAThread();