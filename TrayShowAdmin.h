#pragma once

#include <windows.h>
DWORD WINAPI TSAThreadProc(void *p);

void InitTrayShowAdmin();
BOOL ProcessTrayShowAdmin(BOOL bShowTray, BOOL bBalloon);
void CloseTrayShowAdmin();
BOOL StartTSAThread();