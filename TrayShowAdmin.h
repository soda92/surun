#pragma once

DWORD WINAPI TSAThreadProc(void* p);

void InitTrayShowAdmin();
BOOL ProcessTrayShowAdmin(BOOL bShowTray,BOOL bBalloon);
void CloseTrayShowAdmin();
BOOL StartTSAThread();