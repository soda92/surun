// clang-format off
//go:build ignore
// clang-format on
//////////////////////////////////////////////////////////////////////////////
//
// This source code is part of SuRun
//
// Some sources in this project evolved from Microsoft sample code, some from
// other free sources. The Shield Icons are taken from Windows XP Service Pack
// 2 (xpsp2res.dll)
//
// Feel free to use the SuRun sources for your liking.
//
//                                (c) Kay Bruns (http://kay-bruns.de), 2007-15
//////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0A00
#define WINVER 0x0A00

#include <windows.h>
#include <aclapi.h>
#include <psapi.h>
#include <shlwapi.h>
#include <tchar.h>
#include <userenv.h>

#include "WinStaDesk.h"
#include "DBGTrace.h"
#include "Helpers.h"
#include "ResStr.h"
#include "Setup.h"



#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Userenv.lib")

//////////////////////////////////////////////////////////////////////////////
//
// WindowStation Desktop Names:
//
//////////////////////////////////////////////////////////////////////////////
BOOL GetWinStaName(LPTSTR WinSta, DWORD ccWinSta) {
  return GetUserObjectInformation(GetProcessWindowStation(), UOI_NAME, WinSta,
                                  ccWinSta, &ccWinSta);
}

BOOL GetDesktopName(LPTSTR DeskName, DWORD ccDeskName) {
  return GetUserObjectInformation(GetThreadDesktop(GetCurrentThreadId()),
                                  UOI_NAME, DeskName, ccDeskName, &ccDeskName);
}

//////////////////////////////////////////////////////////////////////////////
//
// The following has evolved taken from Keith Browns cmdasuser
//
// http://www.develop.com/kbrown
//
//////////////////////////////////////////////////////////////////////////////

// for some reason, winuser.h doesn't define these useful aliases
#ifndef WINSTA_ALL_ACCESS
#define WINSTA_ALL_ACCESS 0x0000037F
#endif

#ifndef DESKTOP_ALL_ACCESS
#define DESKTOP_ALL_ACCESS 0x00001FF
#endif

//////////////////////////////////////////////////////////////////////////////
//
// The following has evolved taken from Keith Browns cmdasuser
//
// http://www.develop.com/kbrown
//
//////////////////////////////////////////////////////////////////////////////

// for some reason, winuser.h doesn't define these useful aliases
#ifndef WINSTA_ALL_ACCESS
#define WINSTA_ALL_ACCESS 0x0000037F
#endif

#ifndef DESKTOP_ALL_ACCESS
#define DESKTOP_ALL_ACCESS 0x00001FF
#endif

void SetProcWinStaDesk(LPCTSTR WinSta, LPCTSTR Desk) {
  if (WinSta) {
    HWINSTA hws = WinSta ? OpenWindowStation(WinSta, 0, MAXIMUM_ALLOWED) : 0;
    if (!hws) {
      DBGTrace2("OpenWindowStation(%s) failed: %s", WinSta,
                GetLastErrorNameStatic());
    } else {
      if (!SetProcessWindowStation(hws))
        DBGTrace1("SetProcessWindowStation failed: %s",
                  GetLastErrorNameStatic());
      CloseWindowStation(hws);
    }
  }
  if (Desk) {
    HDESK hd = OpenDesktop(Desk, 0, 0, MAXIMUM_ALLOWED);
    if (!hd) {
      DBGTrace2("OpenDesktop(%s) failed: %s", Desk, GetLastErrorNameStatic());
    } else {
      if (!SetThreadDesktop(hd))
        DBGTrace1("SetThreadDesktop failed: %s", GetLastErrorNameStatic());
      CloseDesktop(hd);
    }
  }
}

void SetAccessToWinDesk(HANDLE htok, LPCTSTR WinSta, LPCTSTR Desk,
                        BOOL bGrant) {
  PSID psidLogonSession = GetLogonSid(htok);
  if (!psidLogonSession)
    return;
  HWINSTA hws = WinSta ? OpenWindowStation(WinSta, 0, MAXIMUM_ALLOWED) : 0;
  if (WinSta && (!hws)) {
    DBGTrace2("OpenWindowStation(%s) failed: %s", WinSta,
              GetLastErrorNameStatic());
  } else {
    HWINSTA hwss = NULL;
    if (hws) {
      PSECURITY_DESCRIPTOR pSD = NULL;
      PACL pOldDACL = NULL, pNewDACL = NULL;
      // get the existing DACL, with enough extra space for adding a couple more
      // aces
      GetSecurityInfo(hws, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, 0, 0,
                      &pOldDACL, 0, &pSD);
      // Initialize an EXPLICIT_ACCESS structure for an ACE.
      EXPLICIT_ACCESS ea[2] = {0};
      // grant the logon session all access to winsta0
      ea[1].grfAccessPermissions = WINSTA_ALL_ACCESS;
      ea[1].grfAccessMode = bGrant ? SET_ACCESS : REVOKE_ACCESS;
      ea[1].grfInheritance = NO_INHERITANCE;
      ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
      ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
      ea[1].Trustee.ptstrName = (LPTSTR)psidLogonSession;
      // grant the logon session all access to any new desktops created in
      // winsta0 by adding an inherit-only ace
      ea[0].grfAccessPermissions = DESKTOP_ALL_ACCESS;
      ea[0].grfAccessMode = bGrant ? SET_ACCESS : REVOKE_ACCESS;
      ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT | INHERIT_ONLY;
      ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
      ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
      ea[0].Trustee.ptstrName = (LPTSTR)psidLogonSession;
      // Create a new ACL that merges the new ACE into the existing DACL.
      DWORD err = SetEntriesInAcl(2, ea, pOldDACL, &pNewDACL);
      if (err != ERROR_SUCCESS) {
        DBGTrace1("SetEntriesInAcl failed:%s", GetErrorNameStatic(err));
      } else {
        // apply changes
        err = SetSecurityInfo(hws, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION,
                              0, 0, pNewDACL, 0);
        if (err)
          DBGTrace1("SetSecurityInfo failed %s", GetErrorNameStatic(err));
      }
      LocalFree((HLOCAL)pNewDACL);
      LocalFree((HLOCAL)pSD);
      // We need to set hws to our WindowsStation to make Desk accessible to us
      hwss = GetProcessWindowStation();
      SetProcessWindowStation(hws);
    }
    HDESK hd = OpenDesktop(Desk, 0, 0, MAXIMUM_ALLOWED);
    if (!hd) {
      DBGTrace2("OpenDesktop(%s) failed: %s", Desk, GetLastErrorNameStatic());
    } else {
      PSECURITY_DESCRIPTOR pSD = NULL;
      PACL pOldDACL = NULL, pNewDACL = NULL;
      // get the existing DACL, with enough extra space for adding a couple more
      // aces
      GetSecurityInfo(hd, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, NULL,
                      NULL, &pOldDACL, NULL, &pSD);
      // Initialize an EXPLICIT_ACCESS structure for an ACE.
      EXPLICIT_ACCESS ea = {0};
      // grant the logon session all access to the default desktop
      ea.grfAccessPermissions = DESKTOP_ALL_ACCESS;
      ea.grfAccessMode = bGrant ? SET_ACCESS : REVOKE_ACCESS;
      ea.grfInheritance = NO_INHERITANCE;
      ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
      ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
      ea.Trustee.ptstrName = (LPTSTR)psidLogonSession;
      // Create a new ACL that merges the new ACE into the existing DACL.
      DWORD err = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
      if (err != ERROR_SUCCESS) {
        DBGTrace1("SetEntriesInAcl failed:%s", GetErrorNameStatic(err));
      } else {
        // apply changes
        err = SetSecurityInfo(hd, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION,
                              0, 0, pNewDACL, 0);
        if (err)
          DBGTrace1("SetSecurityInfo failed %s", GetErrorNameStatic(err));
      }
      LocalFree((HLOCAL)pNewDACL);
      LocalFree((HLOCAL)pSD);
      CloseDesktop(hd);
    }
    if (hwss)
      SetProcessWindowStation(hwss);
    if (hws)
      CloseWindowStation(hws);
  }
  free(psidLogonSession);
}

void GrantUserAccessToDesktop(HDESK hDesk) {
  PSID UserSID = 0;
  DWORD SidLen = 0;
  GetUserObjectInformation(hDesk, UOI_USER_SID, UserSID, SidLen, &SidLen);
  UserSID = LocalAlloc(LPTR, SidLen);
  if (!GetUserObjectInformation(hDesk, UOI_USER_SID, UserSID, SidLen, &SidLen))
    DBGTrace1("GetUserObjectInformation failed:%s", GetLastErrorNameStatic());

  PSECURITY_DESCRIPTOR pSD = NULL;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  // get the existing DACL, with enough extra space for adding a couple more
  // aces
  GetSecurityInfo(hDesk, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, NULL,
                  NULL, &pOldDACL, NULL, &pSD);
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  EXPLICIT_ACCESS ea = {0};
  // grant the logon session all access to the default desktop
  ea.grfAccessPermissions = DESKTOP_ALL_ACCESS;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea.Trustee.ptstrName = (LPTSTR)UserSID;
  // Create a new ACL that merges the new ACE into the existing DACL.
  DWORD err = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (err != ERROR_SUCCESS) {
    DBGTrace1("SetEntriesInAcl failed:%s", GetErrorNameStatic(err));
  } else {
    // apply changes
    err = SetSecurityInfo(hDesk, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, 0,
                          0, pNewDACL, 0);
    if (err)
      DBGTrace1("SetSecurityInfo failed %s", GetErrorNameStatic(err));
  }
  LocalFree(UserSID);
  LocalFree((HLOCAL)pNewDACL);
  LocalFree((HLOCAL)pSD);
}

void DenyUserAccessToDesktop(HDESK hDesk) {
  PSID UserSID = 0;
  DWORD SidLen = 0;
  GetUserObjectInformation(hDesk, UOI_USER_SID, UserSID, SidLen, &SidLen);
  UserSID = LocalAlloc(LPTR, SidLen);
  if (!GetUserObjectInformation(hDesk, UOI_USER_SID, UserSID, SidLen, &SidLen))
    DBGTrace1("GetUserObjectInformation failed:%s", GetLastErrorNameStatic());

  PSECURITY_DESCRIPTOR pSD = NULL;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  // get the existing DACL, with enough extra space for adding a couple more
  // aces
  GetSecurityInfo(hDesk, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, NULL,
                  NULL, &pOldDACL, NULL, &pSD);
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  EXPLICIT_ACCESS ea = {0};
  // grant the logon session all access to the default desktop
  ea.grfAccessPermissions = DESKTOP_ALL_ACCESS;
  ea.grfAccessMode = REVOKE_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea.Trustee.ptstrName = (LPTSTR)UserSID;
  // Create a new ACL that merges the new ACE into the existing DACL.
  DWORD err = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (err != ERROR_SUCCESS) {
    DBGTrace1("SetEntriesInAcl failed:%s", GetErrorNameStatic(err));
  } else {
    // apply changes
    err = SetSecurityInfo(hDesk, SE_WINDOW_OBJECT, DACL_SECURITY_INFORMATION, 0,
                          0, pNewDACL, 0);
    if (err)
      DBGTrace1("SetSecurityInfo failed %s", GetErrorNameStatic(err));
  }
  LocalFree(UserSID);
  LocalFree((HLOCAL)pNewDACL);
  LocalFree((HLOCAL)pSD);
}

CRunOnNewDeskTop *g_RunOnNewDesk = NULL;

//////////////////////////////////////////////////////////////////////////////
//
// CRunOnNewDeskTop:
//   create and Switch to Desktop, switch back when Object is deleted
//
//////////////////////////////////////////////////////////////////////////////

CRunOnNewDeskTop::CRunOnNewDeskTop(LPCTSTR WinStaName, LPCTSTR DeskName,
                                   LPCTSTR UserDesk, bool bCreateBkWnd,
                                   bool bFadeIn) {
  m_hDeskSwitch = NULL;
  m_hwinstaUser = NULL;
  m_bOk = FALSE;
  // Get current WindowStation and Desktop
  m_hwinstaSave = GetProcessWindowStation();
  m_hdeskSave = GetThreadDesktop(GetCurrentThreadId());
  m_hdeskUser = m_hdeskSave;
  TCHAR wsn[MAX_PATH] = {0};
  TCHAR dtn[MAX_PATH] = {0};
  DWORD LenW = MAX_PATH;
  DWORD LenD = MAX_PATH;
  GetUserObjectInformation(m_hwinstaSave, UOI_NAME, wsn, LenW, &LenW);
  GetUserObjectInformation(m_hdeskSave, UOI_NAME, dtn, LenD, &LenD);
  // Set new WindowStation (WinStaName)
  if ((wsn[0] == '\0') || _tcsicmp(wsn, WinStaName)) {
    m_hwinstaUser = OpenWindowStation(WinStaName, FALSE, MAXIMUM_ALLOWED);
    if (m_hwinstaUser == NULL) {
      DBGTrace1("CRunOnNewDeskTop::OpenWindowStation failed: %s",
                GetLastErrorNameStatic());
      SECURITY_ATTRIBUTES saWinSta = {sizeof(saWinSta), NULL, TRUE};
      m_hwinstaUser = CreateWindowStation(
          WinStaName, 0,
          WINSTA_ACCESSCLIPBOARD | WINSTA_ACCESSGLOBALATOMS |
              WINSTA_CREATEDESKTOP | WINSTA_ENUMDESKTOPS | WINSTA_ENUMERATE |
              WINSTA_EXITWINDOWS | WINSTA_READATTRIBUTES | WINSTA_READSCREEN |
              WINSTA_WRITEATTRIBUTES,
          &saWinSta);
      if (m_hwinstaUser == NULL) {
        DBGTrace1("CRunOnNewDeskTop::CreateWindowStation failed: %s",
                  GetLastErrorNameStatic());
        return;
      }
    }
    if (!SetProcessWindowStation(m_hwinstaUser))
      DBGTrace1("CRunOnNewDeskTop::SetProcessWindowStation failed: %s",
                GetLastErrorNameStatic());
  }
  // Create a new Desktop
  LenD = MAX_PATH;
  GetUserObjectInformation(m_hdeskSave, UOI_NAME, dtn, LenD, &LenD);
  if ((dtn[0] == TCHAR('\0')) || _tcsicmp(dtn, DeskName)) {
    // saDesktop.lpSecurityDescriptor allows access to m_hdeskUser only for the
    // current user
    SECURITY_ATTRIBUTES saDesktop = {sizeof(saDesktop), GetUserAccessSD(),
                                     TRUE};
    m_hdeskUser = CreateDesktop(
        DeskName, 0, 0, 0,
        DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE |
            DESKTOP_HOOKCONTROL | DESKTOP_READOBJECTS | DESKTOP_SWITCHDESKTOP |
            DESKTOP_WRITEOBJECTS,
        &saDesktop);
    if (m_hdeskUser == NULL) {
      DBGTrace1("CRunOnNewDeskTop::CreateDesktop failed: %s",
                GetLastErrorNameStatic());
      LocalFree(saDesktop.lpSecurityDescriptor);
      CleanUp();
      return;
    }
    LocalFree(saDesktop.lpSecurityDescriptor);
  }
  // Open a handle to the user Desktop
  m_hDeskSwitch = OpenDesktop(UserDesk, 0, FALSE, DESKTOP_SWITCHDESKTOP);
  if (!m_hDeskSwitch) {
    DBGTrace1("CRunOnNewDeskTop::OpenDesktop failed: %s",
              GetLastErrorNameStatic());
    return;
  }
  // Set Interactive Desktop as current Desktop to get the Desktop Bitmap
  if (!SetThreadDesktop(m_hDeskSwitch))
    DBGTrace1("CRunOnNewDeskTop::SetThreadDesktop(m_hDeskSwitch) failed!: %s",
              GetLastErrorNameStatic());
  // Create Background Bitmap
  if (bCreateBkWnd)
    m_Screen.Init();
  // set m_hdeskUser as the Threads Desktop
  if (!SetThreadDesktop(m_hdeskUser)) {
    DBGTrace("CRunOnNewDeskTop::SetThreadDesktop failed!");
    CleanUp();
    return;
  }
  // Show Desktop Background
  if (bCreateBkWnd)
    m_Screen.Show(bFadeIn);
  m_bOk = TRUE;
}

BOOL CRunOnNewDeskTop::SwitchToOwnDesk() {
  BOOL bRet = SwitchDesktop(m_hdeskUser);
  // Switch to the new Desktop
  if (!bRet)
    DBGTrace1("CRunOnNewDeskTop::SwitchDesktop failed: %s",
              GetLastErrorNameStatic());
  m_Screen.FadeIn();
  return bRet;
}

void CRunOnNewDeskTop::SwitchToUserDesk() {
  for (; m_hDeskSwitch; Sleep(100)) {
    // Switch to the new Desktop
    if (SwitchDesktop(m_hDeskSwitch))
      return;
    if (GetLastError() == ERROR_INVALID_HANDLE)
      m_hDeskSwitch = 0;
    DBGTrace1("CRunOnNewDeskTop::SwitchDesktop failed: %s",
              GetLastErrorNameStatic());
  }
}

void CRunOnNewDeskTop::FadeOut() { m_Screen.FadeOut(); }

void CRunOnNewDeskTop::CleanUp() {
  // GrantUserAccessToDesktop(m_hDeskSwitch);
  // Switch back to the interactive Desktop
  SwitchToUserDesk();
  if (m_hDeskSwitch) {
    if (!CloseDesktop(m_hDeskSwitch))
      DBGTrace1("CRunOnNewDeskTop: CloseDesktop failed: %s",
                GetLastErrorNameStatic());
    m_hDeskSwitch = NULL;
  }
  // Delete Background Window
  if (m_bOk)
    m_Screen.Done();
  // Set the previous Window Station
  if (m_hwinstaUser) {
    if (!SetProcessWindowStation(m_hwinstaSave))
      DBGTrace1("CRunOnNewDeskTop: SetProcessWindowStation failed: %s",
                GetLastErrorNameStatic());
    if (!CloseWindowStation(m_hwinstaUser))
      DBGTrace1("CRunOnNewDeskTop: CloseWindowStation failed: %s",
                GetLastErrorNameStatic());
    m_hwinstaUser = 0;
  }
  // Set the previous Desktop
  if (m_hdeskUser != m_hdeskSave) {
    if (!SetThreadDesktop(m_hdeskSave))
      DBGTrace1("CRunOnNewDeskTop: SetThreadDesktop failed: %s",
                GetLastErrorNameStatic());
    if (!CloseDesktop(m_hdeskUser))
      DBGTrace1("CRunOnNewDeskTop: CloseDesktop failed: %s",
                GetLastErrorNameStatic());
    m_hdeskUser = m_hdeskSave = 0;
  }
  m_bOk = FALSE;
}

CRunOnNewDeskTop::~CRunOnNewDeskTop() { CleanUp(); }

bool CRunOnNewDeskTop::IsValid() { return m_bOk; };

// LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo )
// {
//   DBGTrace("FATAL: Exception in SuRun!");
//   DeleteSafeDesktop(0);
//   return EXCEPTION_EXECUTE_HANDLER;
// }

HANDLE g_WatchDogEvent = NULL;
HANDLE g_WatchDogProcess = NULL;
UINT_PTR g_WatchDogTimer = 0;

// #define WM_LOGONNOTIFY      0x004C
// #define LOCK_WORKSTATION    5
// #define UNLOCK_WORKSTATION  6
//
// static BOOL CALLBACK GetSASWndProc(HWND w,LPARAM p)
//{
//   TCHAR cn[MAX_PATH]={0};
//   GetClassName(w,cn,MAX_PATH);
//   if (_tcscmp(cn,_T("SAS window class"))==0)
//   {
//     *((HWND*)p)=w;
//     return FALSE;
//   }
//   return TRUE;
// }
//
// void LockWndSta()
//{
//   SetProcWinStaDesk(0,L"Winlogon");
//   HWND w=0;
//   EnumWindows(GetSASWndProc,(LPARAM)&w);
//   if (w==0)
//     return;
//   PostMessage(w,WM_LOGONNOTIFY,LOCK_WORKSTATION,0);
// }
//
// void UnLockWndSta()
//{
//   SetProcWinStaDesk(0,L"Winlogon");
//   HWND w=0;
//   EnumWindows(GetSASWndProc,(LPARAM)&w);
//   if (w==0)
//     return;
//   PostMessage(w,WM_LOGONNOTIFY,UNLOCK_WORKSTATION,0);
// }

// callback function for window enumeration
static BOOL CALLBACK CloseAppEnum(HWND hwnd, LPARAM lParam) {
  // no top level window, or invisible?
  if ((GetWindow(hwnd, GW_OWNER)) || (!IsWindowVisible(hwnd)))
    return TRUE;
  TCHAR s[4096] = {0};
  if ((!InternalGetWindowText(hwnd, s, countof(s))) || (s[0] == 0))
    return TRUE;
  DWORD dwID;
  GetWindowThreadProcessId(hwnd, &dwID);
  if (dwID == (DWORD)lParam) {
    DBGTrace1("Closing Window %x", hwnd);
    PostMessage(hwnd, WM_CLOSE, 0, 0);
  }
  return TRUE;
}

BOOL WeMustClose() {
  HWND w = GetForegroundWindow();
  if (!w)
    return FALSE;
  DWORD pid = 0;
  if ((!GetWindowThreadProcessId(w, &pid)) || (!pid)) {
    DBGTrace3(
        "SuRun GUI must be closed (GetWindowThreadProcessId(%x,%d) failed %s)",
        w, pid, GetLastErrorNameStatic());
    return TRUE;
  }
  if (pid == GetCurrentProcessId())
    return FALSE;
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProcess) {
    DBGTrace2("SuRun GUI must be closed (OpenProcess(%d) failed %s)", pid,
              GetLastErrorNameStatic());
    return TRUE;
  }
  DWORD d;
  HMODULE hMod;
  TCHAR f1[MAX_PATH];
  TCHAR f2[MAX_PATH];
  if (!GetProcessFileName(f1, MAX_PATH))
    return CloseHandle(hProcess), TRUE;
  if (!EnumProcessModules(hProcess, &hMod, sizeof(hMod), &d))
    return CloseHandle(hProcess), TRUE;
  if (GetModuleFileNameEx(hProcess, hMod, f2, MAX_PATH) == 0)
    return CloseHandle(hProcess), TRUE;
  CloseHandle(hProcess);
  if (_tcsicmp(f1, f2) != 0) {
    DBGTrace2("SuRun GUI must be closed (\"%s\"!=\"%s\")", f1, f2);
    return TRUE;
  }
  return FALSE;
}

static VOID CALLBACK WDTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                                 DWORD dwTime) {
  UNREFERENCED_PARAMETER(hwnd);
  UNREFERENCED_PARAMETER(uMsg);
  UNREFERENCED_PARAMETER(idEvent);
  UNREFERENCED_PARAMETER(dwTime);
  if (g_WatchDogEvent) {
    SetEvent(g_WatchDogEvent);
    if (WeMustClose()) {
      DBGTrace("SuRun GUI must be closed");
      KillTimer(0, g_WatchDogTimer);
      EnumWindows(CloseAppEnum, (LPARAM)GetCurrentProcessId());
    }
  } else
    KillTimer(0, g_WatchDogTimer);
}

bool CreateSafeDesktop(LPTSTR WinSta, LPCTSTR UserDesk, bool BlurDesk,
                       bool bFade) {
  DeleteSafeDesktop(false);
  //  if (IsLocalSystem())
  //    SetUnhandledExceptionFilter(ExceptionFilter);
  // SuRun misuses WinLogons Desktop
  CResStr DeskName((GetUseWinLogonDesk ? L"Winlogon" : L"SRD_%04x"),
                   GetTickCount());
  // Create Desktop
  CRunOnNewDeskTop *rond =
      new CRunOnNewDeskTop(WinSta, DeskName, UserDesk, BlurDesk, bFade);
  g_RunOnNewDesk = rond;
  if ((!g_RunOnNewDesk) || (!g_RunOnNewDesk->IsValid())) {
    DeleteSafeDesktop(false);
    return false;
  }
  // Start watchdog process:
  PROCESS_INFORMATION pi = {0};
  g_WatchDogEvent = CreateEvent(0, 1, 0, WATCHDOG_EVENT_NAME);
  g_WatchDogTimer = SetTimer(0, 1, 250, WDTimerProc);
  if (g_WatchDogEvent) {
    TCHAR SuRunExe[4096] = {0};
    GetSystemWindowsDirectory(SuRunExe, 4096);
    PathAppend(SuRunExe, L"SuRun.exe");
    PathQuoteSpaces(SuRunExe);
    _stprintf(&SuRunExe[_tcslen(SuRunExe)], L" /WATCHDOG %s %s %d",
              (LPCTSTR)DeskName, UserDesk, GetCurrentProcessId());
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    TCHAR WinstaDesk[MAX_PATH];
    _stprintf(WinstaDesk, _T("%s\\%s"), WinSta, UserDesk);
    si.lpDesktop = WinstaDesk;
    if (CreateProcess(NULL, (LPTSTR)SuRunExe, NULL, NULL, FALSE,
                      CREATE_SUSPENDED, NULL, NULL, &si, &pi))
      g_WatchDogProcess = pi.hProcess;
    else
      DBGTrace("CreateSafeDesktop error could not create WatchDog Process");
  } else
    DBGTrace2("CreateEvent(%s) failed: %s", WATCHDOG_EVENT_NAME,
              GetLastErrorNameStatic());
  if (!rond->SwitchToOwnDesk()) {
    DeleteSafeDesktop(false);
    return false;
  }
  if (pi.hThread) {
    SetThreadPriority(pi.hThread, THREAD_PRIORITY_IDLE);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
  }
  //  if (_winmajor>=6)
  //  {
  //    typedef HRESULT (WINAPI* Win32DwmEnableComposition)(UINT);
  //    static Win32DwmEnableComposition dwmenablecomposition=NULL;
  //    if (!dwmenablecomposition)
  //    {
  //      HINSTANCE dwmapi=LoadLibrary(_T("dwmapi.dll"));
  //      if (dwmapi)
  //        dwmenablecomposition=(Win32DwmEnableComposition)GetProcAddress(dwmapi,"DwmEnableComposition");
  //    }
  //    if(dwmenablecomposition)
  //    {
  //      HRESULT hr=dwmenablecomposition(1);
  //      if (hr!=S_OK)
  //        DBGTrace1("DwmEnableComposition failed: %s",GetErrorNameStatic(hr));
  //    }else
  //      DBGTrace("Could not link to DwmEnableComposition");
  //  }
  //  LockWndSta();
  return true;
}

void DeleteSafeDesktop(bool bFade) {
  //  UnLockWndSta();
  if (g_WatchDogProcess) {
    TerminateProcess(g_WatchDogProcess, 0);
    CloseHandle(g_WatchDogProcess);
  }
  g_WatchDogProcess = 0;
  if (g_RunOnNewDesk && bFade)
    g_RunOnNewDesk->FadeOut();
  if (g_RunOnNewDesk)
    delete g_RunOnNewDesk;
  g_RunOnNewDesk = NULL;
  if (g_WatchDogEvent)
    CloseHandle(g_WatchDogEvent);
  g_WatchDogEvent = NULL;
  if (g_WatchDogTimer)
    KillTimer(0, g_WatchDogTimer);
  g_WatchDogTimer = 0;
  //   if (IsLocalSystem())
  //     SetUnhandledExceptionFilter(NULL);
}

#ifdef _DEBUG
int TestBS() {
  CreateSafeDesktop(_T("WinSta0"), _T("Default"), true, true);
  DWORD t = GetTickCount();
  while (GetTickCount() - t < 6000)
    g_RunOnNewDesk->m_Screen.MsgLoop();
  DeleteSafeDesktop(true);
  ExitProcess(0);
  // return 1;
}
#endif //_DEBUG
