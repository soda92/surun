// clang-format off
//go:build ignore
// clang-format on
#define _WIN32_IE 0x0600
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>
#include <LMAccess.h>
#include <TCHAR.h>
#include <stdio.h>

#include "DBGTrace.h"
#include "IsAdmin.h"
#include "LSALogon.h"
#include "helpers.h"
#include "resource.h"
#include "resstr.h"
#include "service.h"
#include "setup.h"



#pragma comment(lib, "advapi32.lib")

#define CLASSNAME _T("SuRunTrayShowAdminClass")
#define MENU_SURUNSETUP WM_USER + 903

extern RUNDATA g_RunData;

bool g_BallonTips = 1;
BOOL g_ForegroundWndIsAdmin = -1;
HWND g_FgWnd = 0;
HINSTANCE g_hInstance = 0;
TCHAR g_User[MAX_PATH * 2 + 2] = {0};
NOTIFYICONDATA g_NotyData = {0};

struct {
  DWORD CurPID;
  TCHAR CurUserName[UNLEN + GNLEN + 2];
  BOOL CurUserIsadmin;
} g_TSAData = {0};

DWORD g_TSAPID = 0;
BOOL g_TSAThreadRunning = FALSE;
HANDLE g_TSAEvent = 0;

// TSAThreadProc is called from Service!
DWORD WINAPI TSAThreadProc(void *p) {
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, (DWORD)(DWORD_PTR)p);
  if (!hProc)
    return 0;
  struct {
    DWORD CurPID;
    TCHAR CurUserName[UNLEN + GNLEN + 2];
    BOOL CurUserIsadmin;
  } TSAData = {0};
  BOOL TSAThreadRunning = TRUE;
  WriteProcessMemory(hProc, BASE_ADDR(&g_TSAThreadRunning, hProc),
                     &TSAThreadRunning, sizeof(g_TSAThreadRunning), 0);
  HANDLE hEvent = 0;
  SIZE_T s;
  ReadProcessMemory(hProc, BASE_ADDR(&g_TSAEvent, hProc), &hEvent,
                    sizeof(HANDLE), &s);
  DuplicateHandle(hProc, hEvent, GetCurrentProcess(), &hEvent, 0, FALSE,
                  DUPLICATE_SAME_ACCESS);
  EnablePrivilege(SE_DEBUG_NAME);
  for (;;) {
    if ((WaitForSingleObject(hProc, 0) == WAIT_OBJECT_0) ||
        (WaitForSingleObject(hEvent, INFINITE) != WAIT_OBJECT_0) ||
        (!ReadProcessMemory(hProc, BASE_ADDR(&g_TSAPID, hProc), &TSAData.CurPID,
                            sizeof(DWORD), &s)) ||
        (sizeof(DWORD) != s))
      return CloseHandle(hProc), 0;
    // Special case: WM_QUERYENDSESSION
    if (TSAData.CurPID == -1) {
      // ToDo:
    } else
      // Special case: WM_ENDSESSION
      if (TSAData.CurPID == -2) {
        HANDLE hToken = 0;
        if ((_winmajor < 6) &&
            OpenProcessToken(hProc, TOKEN_ALL_ACCESS, &hToken)) {
          SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
          DeleteTempAdminToken(hToken);
          TerminateAllSuRunnedProcesses(hToken);
          SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
          CloseHandle(hToken);
        }
      } else {
        HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, TRUE, TSAData.CurPID);
        if (h) {
          _tcscpy(TSAData.CurUserName, _T("unknown"));
          TSAData.CurUserIsadmin = FALSE;
          HANDLE hTok = 0;
          if (OpenProcessToken(h, TOKEN_QUERY | TOKEN_DUPLICATE, &hTok)) {

            GetTokenUserName(hTok, TSAData.CurUserName);
            TSAData.CurUserIsadmin = IsAdmin(hTok);
            CloseHandle(hTok);
          } else
            DBGTrace1("OpenProcessToken failed: %s", GetLastErrorNameStatic());
          CloseHandle(h);
        } // else
        // DBGTrace2("OpenProcess(%d) failed:
        // %s",TSAData.CurPID,GetLastErrorNameStatic());
      }
    WriteProcessMemory(hProc, BASE_ADDR(&g_TSAData, hProc), &TSAData,
                       sizeof(g_TSAData), &s);
    ResetEvent(hEvent);
  }
}

BOOL StartTSAThread() {
  if (g_TSAThreadRunning)
    return TRUE;
  g_TSAPID = 0;
  zero(g_TSAData);
  g_RunData.KillPID = 0xFFFFFFFF;
  _tcscpy(g_RunData.cmdLine, _T("/TSATHREAD"));
  g_RetVal = RETVAL_WAIT;
  HANDLE hPipe =
      CreateFile(ServicePipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if (hPipe == INVALID_HANDLE_VALUE)
    return FALSE;
  g_TSAEvent = CreateEvent(0, 1, 0, 0);
  DWORD n = 0;
  WriteFile(hPipe, &g_RunData, sizeof(RUNDATA), &n, 0);
  CloseHandle(hPipe);
  Sleep(10);
  for (n = 0; (!g_TSAThreadRunning) && (n < 100); n++)
    Sleep(100);
  return g_TSAThreadRunning;
}

static void ShutDownSuRunProcesses() {
  if (!StartTSAThread())
    return;
  g_TSAPID = (DWORD)-2;
  SetEvent(g_TSAEvent);
  for (; g_TSAPID != g_TSAData.CurPID;)
    Sleep(10);
  g_TSAPID = 0;
}

static BOOL ForegroundWndIsAdmin(LPTSTR User, HWND &wnd, LPTSTR WndTitle,
                                 DWORD &CurPID) {
  if ((g_TSAPID == -2) || (g_TSAPID == -1))
    return -1;
  if (!StartTSAThread())
    return -1;
  HWND Wnd = GetForegroundWindow();
  if (!Wnd)
    return -1;
  HWND w = GetParent(Wnd);
  while (w) {
    Wnd = w;
    w = GetParent(Wnd);
  }
  GetWindowThreadProcessId(Wnd, &g_TSAPID);
  if (g_TSAPID != g_TSAData.CurPID) {
    CTimeOut t(1000);
    SetEvent(g_TSAEvent);
    for (; g_TSAPID != g_TSAData.CurPID;) {
      if (t.TimedOut())
        return -1;
      Sleep(10);
    }
  }
  _tcscpy(User, g_TSAData.CurUserName);
  if (!g_TSAData.CurPID)
    return -1;
  wnd = Wnd;
  CurPID = g_TSAData.CurPID;
  if (!InternalGetWindowText(wnd, WndTitle, MAX_PATH))
    if (!GetProcessName(CurPID, WndTitle))
      _stprintf(WndTitle, L"Process %d", CurPID);
  return g_TSAData.CurUserIsadmin;
}

static DWORD LastPID = 0;
static CTimeOut BalloonTimeOut(0);
static bool g_IconShown = FALSE;
static void DisplayIcon() {
  HICON OldIcon = g_NotyData.hIcon;
  TCHAR User[MAX_PATH * 2 + 2] = {0};
  TCHAR WndTxt[MAX_PATH + 1] = {0};
  HWND FgWnd = 0;
  DWORD CurPID = 0;
  BOOL bIsFGAdm = ForegroundWndIsAdmin(User, FgWnd, WndTxt, CurPID);
  if (_tcslen(WndTxt) > 64) {
    WndTxt[61] = '.';
    WndTxt[62] = '.';
    WndTxt[63] = '.';
    WndTxt[64] = 0;
  }
  if (_tcslen(User) > 32) {
    WndTxt[29] = '.';
    WndTxt[30] = '.';
    WndTxt[31] = '.';
    WndTxt[32] = 0;
  }
  BOOL bNoUpd = (g_ForegroundWndIsAdmin == bIsFGAdm) &&
                (_tcscmp(g_User, User) == 0) && (g_FgWnd == FgWnd);
  if (BalloonTimeOut.TimedOut() && (CurPID != LastPID))
    LastPID = 0;
  if ((BalloonTimeOut.TimedOut() || (CurPID != LastPID)) &&
      (g_NotyData.szInfo[0])) {
    memset(&g_NotyData.szInfoTitle, 0, sizeof(g_NotyData.szInfoTitle));
    memset(&g_NotyData.szInfo, 0, sizeof(g_NotyData.szInfo));
    bNoUpd = FALSE;
  }
  if (bNoUpd)
    return;
  g_FgWnd = FgWnd;
  g_ForegroundWndIsAdmin = bIsFGAdm;
  _tcscpy(g_User, User);
  _stprintf(g_NotyData.szTip, _T("SuRun %s"), GetVersionString());
  BOOL bDiffUser = _tcscmp(User, g_RunData.UserName);
  g_NotyData.dwState = 0;
  if (bIsFGAdm == -1) {
    g_NotyData.hIcon =
        (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_NOWINDOW), IMAGE_ICON,
                         128, 128, LR_DEFAULTCOLOR);
    if (GetHideNormalIcon)
      g_NotyData.dwState = NIS_HIDDEN;
  } else if (bIsFGAdm) {
    g_NotyData.hIcon = (HICON)LoadImage(
        g_hInstance,
        MAKEINTRESOURCE(bDiffUser ? IDI_ADMIN
                                  : (g_CliIsAdmin ? IDI_SHADMIN : IDI_SRADMIN)),
        IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
    if (g_BallonTips && bDiffUser && (CurPID != LastPID)) {
      LastPID = CurPID;
      BalloonTimeOut.Set(20000);
      _stprintf(g_NotyData.szInfoTitle, _T("SuRun %s"), GetVersionString());
      _stprintf(g_NotyData.szInfo, _T("\"%s\"\n\"%s\" - Administrator"), WndTxt,
                User);
    }
    _stprintf(g_NotyData.szTip, _T("\"%s\"\n\"%s\" - Administrator"), WndTxt,
              User);
  } else {
    g_NotyData.hIcon =
        (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_NOADMIN), IMAGE_ICON,
                         128, 128, LR_DEFAULTCOLOR);
    if (g_BallonTips && bDiffUser && (CurPID != LastPID)) {
      LastPID = CurPID;
      BalloonTimeOut.Set(20000);
      _stprintf(g_NotyData.szInfoTitle, _T("SuRun %s"), GetVersionString());
      _stprintf(g_NotyData.szInfo, _T("\"%s\"\n\"%s\" - Standard user"), WndTxt,
                User);
    }
    _stprintf(g_NotyData.szTip, _T("\"%s\"\n\"%s\" - Standard user"), WndTxt,
              User);
    if (GetHideNormalIcon && (!bDiffUser))
      g_NotyData.dwState = NIS_HIDDEN;
  }
  if (!Shell_NotifyIcon(NIM_MODIFY, &g_NotyData)) {
    Shell_NotifyIcon(NIM_ADD, &g_NotyData);
    g_NotyData.uVersion = NOTIFYICON_VERSION;
    Shell_NotifyIcon(NIM_SETVERSION, &g_NotyData);
  }
  g_IconShown = TRUE;
  DestroyIcon(OldIcon);
};

void MenuAdd(HMENU hMenu, LPTSTR Text, int CMD, int Flags) {
  MENUITEMINFO mii;
  ZeroMemory(&mii, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_CHECKMARKS;
  mii.wID = CMD;
  mii.fType = MFT_STRING;
  mii.dwTypeData = Text;
  mii.fState = Flags;
  InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
}

void DisplayMenu(HWND hWnd) {
  bool bBTip = g_BallonTips;
  g_BallonTips = 0;
  if (bBTip) {
    // Close Balloon
    memset(&g_NotyData.szInfoTitle, 0, sizeof(g_NotyData.szInfoTitle));
    memset(&g_NotyData.szInfo, 0, sizeof(g_NotyData.szInfo));
    _stprintf(g_NotyData.szTip, _T("SuRun %s"), GetVersionString());
    if (g_IconShown) {
      Shell_NotifyIcon(NIM_MODIFY, &g_NotyData);
      g_IconShown = FALSE;
    }
  }
  POINT pt;
  GetCursorPos(&pt);
  HMENU hMenu = CreatePopupMenu();
  MenuAdd(hMenu, CResStr(_T("SuRun %s"), GetVersionString()), (UINT)-1,
          MFS_DEFAULT);
  AppendMenu(hMenu, MF_SEPARATOR, (UINT)-1, 0);
  MenuAdd(hMenu, CResStr(IDS_CPLNAME), MENU_SURUNSETUP, 0);
  AllowSetForegroundWindow(GetCurrentProcessId());
  SetForegroundWindow(hWnd);
  switch (TrackPopupMenu(hMenu,
                         TPM_RIGHTALIGN | TPM_RETURNCMD | TPM_NONOTIFY |
                             TPM_VERNEGANIMATION,
                         pt.x - 1, pt.y - 1, 0, hWnd, NULL)) {
  case MENU_SURUNSETUP:
    ShellExecute(0, _T("open"), _T("SuRun.exe"), _T("/SETUP"), 0,
                 SW_SHOWNORMAL);
    break;
  }
  DestroyMenu(hMenu);
  g_BallonTips = bBTip;
};

// int g_HotKeyID=0;

LRESULT CALLBACK WndMainProc(HWND hwnd, UINT message, WPARAM wParam,
                             LPARAM lParam) {
  switch (message) {
  case WM_TIMER:
    break;
  case WM_USER + 1758: {
    switch (lParam) {
    case NIN_BALLOONSHOW:
      break;
    case NIN_BALLOONHIDE:
      // other App active
      break;
    case NIN_BALLOONTIMEOUT:
      // Click on [X] or TimeOut
    case NIN_BALLOONUSERCLICK:
      // Click in Balloon
      break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_CONTEXTMENU:
      DisplayMenu(hwnd);
      return 0;
    case WM_LBUTTONDBLCLK:
      ShellExecute(0, _T("open"), _T("SuRun.exe"), _T("/SETUP"), 0,
                   SW_SHOWNORMAL);
      return 0;
    }
  } break;
  case WM_QUERYENDSESSION: {
    // ToDo: Warn, if SuRun processes are still running.
  } break;
  case WM_ENDSESSION: {
    // ToDo: Warn, if SuRun processes are still running.
    if (_winmajor < 6)
      ShutDownSuRunProcesses();
  } break;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void InitTrayShowAdmin() {
  g_hInstance = GetModuleHandle(0);
  g_CliIsAdmin = FALSE;
  HANDLE hTok = GetShellProcessToken();
  if (hTok) {
    g_CliIsAdmin = IsAdmin(hTok) != 0;
    CloseHandle(hTok);
  }
  WNDCLASS WCLASS = {0};
  WCLASS.hCursor = LoadCursor(0, IDC_ARROW);
  WCLASS.lpszClassName = CLASSNAME;
  WCLASS.hInstance = g_hInstance;
  WCLASS.style = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
  WCLASS.lpfnWndProc = &WndMainProc;
  RegisterClass(&WCLASS);
  g_NotyData.cbSize = sizeof(NOTIFYICONDATA);
  g_NotyData.hWnd = CreateWindowEx(
      WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, CLASSNAME, _T("SuRunTrayShowAdmin"),
      WS_POPUP, 0, 0, 0, 0, 0, 0, g_hInstance, NULL);
  g_NotyData.hIcon =
      (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_NOWINDOW), IMAGE_ICON,
                       128, 128, LR_DEFAULTCOLOR);

  _stprintf(g_NotyData.szTip, _T("SuRun %s"), GetVersionString());
  g_NotyData.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
  g_NotyData.uID = 1;
  g_NotyData.uFlags = NIF_ICON | NIF_TIP | NIF_INFO | NIF_MESSAGE | NIF_STATE;
  g_NotyData.dwState = NIS_HIDDEN;
  g_NotyData.dwStateMask = NIS_HIDDEN;
  g_NotyData.uCallbackMessage = WM_USER + 1758;
  g_NotyData.uTimeout = 10000;
  SetTimer(g_NotyData.hWnd, 1, 333, 0);
}

BOOL ProcessTrayShowAdmin(BOOL bShowTray, BOOL bBalloon) {
  g_BallonTips = bBalloon != 0;
  if (bShowTray) {
    DisplayIcon();
  } else {
    if (g_IconShown)
      Shell_NotifyIcon(NIM_DELETE, &g_NotyData);
    g_IconShown = FALSE;
  }
  MSG msg;
  do {
    if (!GetMessage(&msg, 0, 0, 0))
      return FALSE;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  } while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE));
  return TRUE;
}

void CloseTrayShowAdmin() {
  Shell_NotifyIcon(NIM_DELETE, &g_NotyData);
  DestroyWindow(g_NotyData.hWnd);
}
