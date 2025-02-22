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
#ifdef _DEBUG
// #define _DEBUGSETUP
#endif //_DEBUG

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>
#include <Tlhelp32.h>
#include <dbghelp.h>
#include <lm.h>
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>


#include "DBGTrace.h"
#include "Helpers.h"
#include "IsAdmin.h"
#include "ResStr.h"
#include "Resource.h"
#include "Service.h"
#include "Setup.h"
#include "TrayMsgWnd.h"
#include "UserGroups.h"
#include "WinStaDesk.h"


#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "shlwapi")

#ifdef _WIN64
#pragma comment(lib, "bin/Crypt32x64.lib")
#else //_WIN64
#pragma comment(lib, "bin/Crypt32x86.lib")
#endif //_WIN64

#ifdef _DEBUG
#include "LogonDlg.h"
#include "WatchDog.h"
#endif //_DEBUG

extern RUNDATA g_RunData;

//////////////////////////////////////////////////////////////////////////////
//
// WinMain
//
//////////////////////////////////////////////////////////////////////////////

static void HideAppStartCursor() {
  HWND w = CreateWindow(_TEXT("Static"), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (w) {
    PostMessage(w, WM_QUIT, 0, 0);
    MSG msg;
    GetMessage(&msg, w, WM_QUIT, WM_QUIT);
    DestroyWindow(w);
  }
}

// extern LPTSTR GetSvcName();
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(hPrevInst);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  {
    // Enable DEP
    HMODULE hMod = GetModuleHandle(_T("Kernel32.dll"));
    if (hMod) {
      typedef BOOL(WINAPI * PSETDEP)(DWORD);
#define PROCESS_DEP_ENABLE 0x00000001
      PSETDEP SetProcessDEPPolicy =
          (PSETDEP)GetProcAddress(hMod, "SetProcessDEPPolicy");
      if (SetProcessDEPPolicy)
        SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
    }
  }
  DBGTrace1("SuRun started with (%s)", GetCommandLine());
#ifdef _DEBUG
//   CreateEvent(0,1,0,WATCHDOG_EVENT_NAME);
//   DoWatchDog(L"Winlogon",L"Default",GetCurrentProcessId());
//   if
//   (HasRegistryKeyAccess(_T("MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Controls
//   Folder\\PowerCfg"),_T("SuRunners")))
//     DBGTrace("2 ok");
//   if
//   (HasRegistryKeyAccess(_T("CURRENT_USER\\Software\\SuRun"),_T("SuRunners")))
//     DBGTrace("1 ok");
//   TestLogonDlg();
//   return 0;
#endif //_DEBUG
  switch (GetRegInt(HKLM, SURUNKEY, L"Language", 0)) {
  case 1:
    SetLocale(MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN));
    break;
  case 2:
    SetLocale(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    break;
  case 3:
    SetLocale(MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH));
    break;
  case 4:
    SetLocale(MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH));
    break;
  case 5:
    SetLocale(MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT));
    break;
  case 6:
    SetLocale(MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH));
    break;
  case 7:
    SetLocale(MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE));
    break;
  }
  HideAppStartCursor();
  if (HandleServiceStuff()) {
    DBGTrace("HandleServiceStuff exit");
    return 0;
  }
  if (g_RunData.CliThreadId == GetCurrentThreadId()) {
    DBGTrace("TrayMsgWnd exit");
    // Started from service:
    // Show ToolTip "<Program> is running elevated"...
    TrayMsgWnd(CResStr(IDS_APPNAME), g_RunData.cmdLine, g_RunData.IconId,
               g_RunData.TimeOut);
    return RETVAL_OK;
  }
  zero(g_RunData.cmdLine);
  // ProcessId
  g_RunData.CliProcessId = GetCurrentProcessId();
  // ThreadId
  g_RunData.CliThreadId = GetCurrentThreadId();
  // Session
  ProcessIdToSessionId(g_RunData.CliProcessId, &g_RunData.SessionID);
  // WindowStation
  GetWinStaName(g_RunData.WinSta, countof(g_RunData.WinSta));
  // Desktop
  GetDesktopName(g_RunData.Desk, countof(g_RunData.Desk));
  // UserName
  GetProcessUserName(g_RunData.CliProcessId, g_RunData.UserName);
  // Groups
  g_RunData.Groups = UserIsInSuRunnersOrAdmins();
  // Current Directory
  GetCurrentDirectory(countof(g_RunData.CurDir), g_RunData.CurDir);
  NetworkPathToUNCPath(g_RunData.CurDir);
  bool bRunSetup = FALSE;
  bool bRetPID = FALSE;
  bool bWaitPID = FALSE;
  if (HideSuRun(g_RunData.UserName, g_RunData.Groups))
    g_RunData.beQuiet = TRUE;
  // cmdLine
  {
    LPTSTR args = _tcsdup(PathGetArgs(GetCommandLine()));
    LPTSTR Args = args;
    while (Args[0] == L' ')
      Args++;
    // Parse direct commands:
    while (Args[0] == '/') {
      LPTSTR c = Args;
      Args = PathGetArgs(Args);
      if (Args)
        for (LPTSTR C = Args - 1; C && (*C == ' '); C--)
          *C = 0;
      if (!_wcsicmp(c, L"/QUIET")) {
        g_RunData.beQuiet = TRUE;
      } else if (!_wcsicmp(c, L"/RETPID")) {
        bRetPID = TRUE;
      } else if (!_wcsicmp(c, L"/WAIT")) {
        bWaitPID = TRUE;
      } else if (!_wcsicmp(c, L"/RUNAS")) {
        g_RunData.bRunAs |= 1;
      } else if (!_wcsicmp(c, L"/LOW")) {
        g_RunData.bRunAs |= 2;
      } else if ((!_wcsicmp(c, L"/USER")) && (g_RunData.bRunAs & 1)) {
        c = Args;
        Args = PathGetArgs(Args);
        if (Args)
          for (LPTSTR C = Args - 1; C && (*C == ' '); C--)
            *C = 0;
        wcsncpy(g_RunData.UserName, c, UNLEN + UNLEN);
        g_RunData.bRunAs |= 4;
      } else if (!_wcsicmp(c, L"/SETUP")) {
        bRunSetup = TRUE;
        wcscpy(g_RunData.cmdLine, L"/SETUP");
        break;
      } else if (!_wcsicmp(c, L"/EmptyRecycleBin")) {
        if (g_RunData.Groups & IS_IN_ADMINS) {
          DBGTrace("SuRun: g_RunData.Groups&IS_IN_ADMINS Exit");
          return SHEmptyRecycleBin(0, 0, 0) == S_OK ? RETVAL_OK
                                                    : RETVAL_ACCESSDENIED;
        }
        wcscpy(g_RunData.cmdLine, L"/EmptyRecycleBin");
        g_RunData.KillPID = 0xFFFFFFFF;
        break;
      } else if (!_wcsicmp(c, L"/TESTAA")) {
        g_RunData.bShlExHook = TRUE;
        // ShellExec-Hook: We must return the PID and TID to fake CreateProcess:
        g_RunData.RetPID = wcstol(Args, 0, 10);
        Args = PathGetArgs(Args);
#ifndef _WIN64
        g_RunData.RetPtr = wcstoul(Args, 0, 16);
#else //_WIN64
        g_RunData.RetPtr = _wcstoui64(Args, 0, 16);
#endif //_WIN64
        Args = PathGetArgs(Args);
        // If we run on a desktop we cannot switch from, bail out!
        HDESK d = OpenInputDesktop(0, 0, 0);
        if (!d)
          g_RunData.bShExNoSafeDesk = TRUE;
        else {
          TCHAR dn[4096] = {0};
          DWORD dnl = 4096;
          if (!GetUserObjectInformation(d, UOI_NAME, dn, dnl, &dnl))
            g_RunData.bShExNoSafeDesk = TRUE;
          else if ((_tcsicmp(dn, _T("Winlogon")) == 0) ||
                   (_tcsicmp(dn, _T("Disconnect")) == 0) ||
                   (_tcsicmp(dn, _T("Screen-saver")) == 0))
            g_RunData.bShExNoSafeDesk = TRUE;
          CloseDesktop(d);
        }
      } else if (!_wcsicmp(c, L"/KILL")) {
        g_RunData.KillPID = wcstol(Args, 0, 10);
        Args = PathGetArgs(Args);
      }
      //       else if (!_wcsicmp(c,L"/CONSPID"))
      //       {
      //         g_RunData.ConsolePID=wcstol(Args,0,10);
      //         Args=PathGetArgs(Args);
      //       }
      else if (!_wcsicmp(c, L"/RESTORE")) {
        if (!(g_RunData.Groups & IS_IN_ADMINS)) {
          SafeMsgBox(0, CBigResStr(IDS_NOIMPORT), CResStr(IDS_APPNAME),
                     MB_ICONSTOP);
          return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST
                                      : RETVAL_ACCESSDENIED;
        }
        g_RunData.KillPID = (DWORD)-1;
        if (Args)
          for (LPTSTR C = Args - 1; C && (*C == 0); C--)
            *C = ' ';
        Args = c;
        break;
      } else if (!_wcsicmp(c, L"/BACKUP")) {
        if (!(g_RunData.Groups & IS_IN_ADMINS)) {
          SafeMsgBox(0, CBigResStr(IDS_NOEXPORT), CResStr(IDS_APPNAME),
                     MB_ICONSTOP);
          return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST
                                      : RETVAL_ACCESSDENIED;
        }
        g_RunData.KillPID = (DWORD)-1;
        if (Args)
          for (LPTSTR C = Args - 1; C && (*C == 0); C--)
            *C = ' ';
        Args = c;
        break;
      } else if (!_wcsicmp(c, L"/SWITCHTO")) {
        g_RunData.KillPID = (DWORD)-1;
        if (Args)
          for (LPTSTR C = Args - 1; C && (*C == 0); C--)
            *C = ' ';
        Args = c;
        wcscpy(g_RunData.cmdLine, Args);
        break;
        //       }else if (!_wcsicmp(c,L"/WHOAMI"))
        //       {
        //         DWORD gr=UserIsInSuRunnersOrAdmins();
        //         return
        //         SafeMsgBox(0,(gr&IS_IN_ADMINS)?L"Admin":((gr&IS_SPLIT_ADMIN)?L"Split
        //         Admin":((gr&IS_IN_SURUNNERS)?L"SuRunner":L"No
        //         Admin")),CResStr(IDS_APPNAME),MB_ICONINFORMATION);
      } else {
        // invalid direct argument
        DBGTrace("SuRun: Invalid usage!");
        return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST : RETVAL_ACCESSDENIED;
      }
    }
    // Convert Command Line
    if (!g_RunData.cmdLine[0]) {
      bool bShellIsadmin = FALSE;
      HANDLE hTok = GetShellProcessToken();
      if (hTok) {
        bShellIsadmin = IsAdmin(hTok) != 0;
        CloseHandle(hTok);
      }
      // If shell is Admin but User is SuRunner, the Shell must be restarted
      if ((!g_RunData.bRunAs) && g_CliIsInSuRunners && bShellIsadmin) {
        // Complain if shell user is an admin!
        SafeMsgBox(0, CResStr(IDS_ADMINSHELL), CResStr(IDS_APPNAME),
                   MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        DBGTrace("SuRun: IDS_ADMINSHELL Exit");
        return RETVAL_ACCESSDENIED;
      }
      // ToDo: use dynamic allocated strings
      if (StrLenW(Args) + StrLenW(g_RunData.CurDir) > 4096 - 64) {
        DBGTrace("SuRun: Strings too long Exit");
        return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST : RETVAL_ACCESSDENIED;
      }
      ResolveCommandLine(Args, g_RunData.CurDir, g_RunData.cmdLine);
      // DBGTrace3("ResolveCommandLine(%s,%s)=
      // %s",Args,g_RunData.CurDir,g_RunData.cmdLine);
    }
    free(args);
  }
  // Usage
  if (!g_RunData.cmdLine[0]) {
    if (!g_RunData.beQuiet)
      SafeMsgBox(0, CBigResStr(IDS_USAGE), CResStr(IDS_APPNAME), MB_ICONSTOP);
    return RETVAL_ACCESSDENIED;
  }
  // Lets go:
  g_RetVal = RETVAL_WAIT;
  HANDLE hPipe = INVALID_HANDLE_VALUE;
  // retry if the pipe is busy: (max 240s)
  CTimeOut to(240000);
  while (!to.TimedOut()) {
    hPipe =
        CreateFile(ServicePipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    DWORD err = GetLastError();
    if (hPipe != INVALID_HANDLE_VALUE) {
      break;
    }
    if ((err == ERROR_FILE_NOT_FOUND) || (err == ERROR_ACCESS_DENIED)) {
      DBGTrace1("Pipe not ok: %s", GetErrorNameStatic(err));
      return RETVAL_ACCESSDENIED;
    }
    DBGTrace2("SuRun CreateFile(%s) failed: %s", ServicePipeName,
              GetErrorNameStatic(err));
    Sleep(250);
  }
  // No Pipe handle: fail!
  if (hPipe == INVALID_HANDLE_VALUE) {
    DBGTrace("Pipe not ok, Exit");
    return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST : RETVAL_ACCESSDENIED;
  }
  DWORD nWritten = 0;
  WriteFile(hPipe, &g_RunData, sizeof(RUNDATA), &nWritten, 0);
  CloseHandle(hPipe);
  // Wait for max 60s for the Password...
  to.Set(60000);
  while ((g_RetVal == RETVAL_WAIT) && (!to.TimedOut()))
    Sleep(20);
  if (bRunSetup) {
    DBGTrace("Setup Exit");
    return g_RetVal;
  }
  switch (g_RetVal) {
  case RETVAL_WAIT:
    DBGTrace("ERROR: SuRun got no response from Service!");
    return ERROR_ACCESS_DENIED;
  case RETVAL_SX_NOTINLIST: // ShellExec->NOT in List
    DBGTrace("Retval RETVAL_SX_NOTINLIST");
    return RETVAL_SX_NOTINLIST;
  case RETVAL_RESTRICT: // Restricted User, may not run App!
    if (!g_RunData.beQuiet)
      SafeMsgBox(
          0,
          CBigResStr(IDS_RUNRESTRICTED, g_RunData.UserName, g_RunData.cmdLine),
          CResStr(IDS_APPNAME), MB_ICONSTOP);
    DBGTrace("Retval RETVAL_RESTRICT");
    return RETVAL_RESTRICT;
  case RETVAL_ACCESSDENIED:
    DBGTrace("Retval RETVAL_ACCESSDENIED");
    if (!g_RunData.beQuiet) {
      SafeMsgBox(0, CResStr(IDS_RUNFAILED, g_RunData.cmdLine),
                 CResStr(IDS_APPNAME), MB_ICONSTOP);
    }
    return RETVAL_ACCESSDENIED;
  case RETVAL_CANCELLED:
    DBGTrace("Retval RETVAL_CANCELLED");
    return RETVAL_CANCELLED;
  case RETVAL_OK: {
    if (bWaitPID) {
      HANDLE hProcess = OpenProcess(SYNCHRONIZE, 0, g_RunData.NewPID);
      if (hProcess) {
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
      } else
        DBGTrace2("SuRun OpenProcess(%d) failed: %s", g_RunData.NewPID,
                  GetLastErrorNameStatic());
    }
    return bRetPID ? g_RunData.NewPID : RETVAL_OK;
  }
  }
  DBGTrace("Returning RETVAL_ACCESSDENIED");
  return RETVAL_ACCESSDENIED;
}
