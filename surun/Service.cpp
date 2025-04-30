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

// This is the main file for the SuRun service. This file handles:
// -SuRun Installation
// -SuRun Uninstallation
// -the Windows Service
// -putting the user to the SuRunners group
// -Terminating a Process for "Restart as admin..."
// -requesting permission/password in the logon session of the user
// -putting the users password to the user process via WriteProcessMemory

#define _WIN32_WINNT 0x0A00
#define WINVER 0x0A00
#include <windows.h>
#include <Aclapi.h>
#include <Psapi.h>
#include <Sddl.h>
#include <Tlhelp32.h>
#include <USERENV.H>
#include <lm.h>
#include <ntsecapi.h>
#include <stdio.h>
#include <tchar.h>

#include "Service.h"
#include "CmdLine.h"
#include "DBGTrace.h"
#include "DynWTSAPI.h"
#include "Helpers.h"
#include "IsAdmin.h"
#include "LSALogon.h"
#include "LSA_laar.h"
#include "LogonDlg.h"
#include "ReqAdmin.h"
#include "ResStr.h"
#include "Resource.h"
#include "Setup.h"
#include "SuRunExt/SuRunExt.h"
#include "SuRunExt/SysMenuHook.h"
#include "TrayShowAdmin.h"
#include "UserGroups.h"
#include "WatchDog.h"
#include "WinStaDesk.h"

inline unsigned int _GetWinVer() {
  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&vi);
  _osplatform = vi.dwPlatformId;
  _winmajor = vi.dwMajorVersion;
  _winminor = vi.dwMinorVersion;
  _osver = (vi.dwBuildNumber) & 0x07fff;
  if (_osplatform != VER_PLATFORM_WIN32_NT)
    _osver |= 0x08000;
  return (_winmajor << 8) + _winminor;
}

#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "PSAPI.lib")

#ifndef _DEBUG

#ifdef _WIN64
#pragma comment(lib, "SuRunExt/ReleaseUx64/SuRunExt.lib")
#else //_WIN64
#ifdef _SR32
#pragma comment(lib, "SuRunExt/ReleaseUsr32/SuRunExt32.lib")
#else //_SR32
#pragma comment(lib, "SuRunExt/ReleaseU/SuRunExt.lib")
#endif //_SR32
#endif //_WIN64

#else //_DEBUG

#ifdef _WIN64
#pragma comment(lib, "SuRunExt/DebugUx64/SuRunExt.lib")
#else //_WIN64
#ifdef _SR32
#pragma comment(lib, "SuRunExt/DebugUsr32/SuRunExt32.lib")
#else //_SR32
#pragma comment(lib, "SuRunExt/DebugU/SuRunExt.lib")
#endif //_SR32
#endif //_WIN64

#endif //_DEBUG

// #ifdef DoDBGTrace
// DWORD g_RunTimes[16]={0};
// LPCTSTR g_RunTimeNames[16]={0};
// DWORD g_nTimes=0;
// #define AddTime(s) { g_RunTimes[g_nTimes]=timeGetTime();
// g_RunTimeNames[g_nTimes++]=_TEXT(s); } #endif //DoDBGTrace
//////////////////////////////////////////////////////////////////////////////
//
//  Globals
//
//////////////////////////////////////////////////////////////////////////////

static SERVICE_STATUS_HANDLE g_hSS = 0;
static SERVICE_STATUS g_ss = {0};
static HANDLE g_hPipe = INVALID_HANDLE_VALUE;
CResStr SvcDisplayText(IDS_SERVICE_NAME);

RUNDATA g_RunData = {0};
TCHAR g_RunPwd[PWLEN] = {0};

int g_RetVal = 0;
bool g_CliIsAdmin = FALSE;

BOOL IsCorrectServiceName(SC_HANDLE hdlSCM, LPTSTR SvcName) {
  TCHAR sn[256] = {0};
  DWORD n = 256;
  return GetServiceDisplayName(hdlSCM, SvcName, sn, &n);
}

LPTSTR GetSvcName() {
  static BOOL bGotSvcName = FALSE;
  static LPTSTR SvcNameID = NULL;
  if (bGotSvcName)
    return SvcNameID;
  SC_HANDLE hdlSCM =
      OpenSCManager(0, 0, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
  if (hdlSCM == 0) {
    DBGTrace1("OpenSCManager failed: %s", GetLastErrorNameStatic());
    return FALSE;
  }
  bGotSvcName = TRUE;
  SvcNameID = L"SuRunSVC";
  if (!IsCorrectServiceName(hdlSCM, SvcNameID)) {
    SvcNameID = L"Super User Run (SuRun) Service";
    if (!IsCorrectServiceName(hdlSCM, SvcNameID)) {
      SvcNameID = L"Us³uga Super User Run (SuRun)";
      if (!IsCorrectServiceName(hdlSCM, SvcNameID)) {
        SvcNameID = L"Servicio de Super User Run (SuRun)";
        if (!IsCorrectServiceName(hdlSCM, SvcNameID)) {
          SvcNameID = L"Service Super User Run (SuRun)";
          if (!IsCorrectServiceName(hdlSCM, SvcNameID))
            SvcNameID = L"SuRunSVC";
        }
      }
    }
  }
  CloseServiceHandle(hdlSCM);
  return SvcNameID;
}

//////////////////////////////////////////////////////////////////////////////
//
//  Fast User Switching
//
//////////////////////////////////////////////////////////////////////////////
static BOOL SwitchToSession(DWORD SessionId) {
  HMODULE hWinSta = LoadLibraryW(L"winsta.dll");
  if (!hWinSta)
    return FALSE;
  typedef BOOL(WINAPI * WSCW)(HANDLE, DWORD, DWORD, PWCHAR, BOOL);
  WSCW WinStationConnectW = (WSCW)GetProcAddress(hWinSta, "WinStationConnectW");
  if (!WinStationConnectW)
    return FALSE;
  BOOL bRet = WinStationConnectW(0, SessionId, (DWORD)-1, L"", TRUE);
  return bRet;
}

static DWORD DoFUS() // Do Fast User Switching
{
  TCHAR UserName[UNLEN + UNLEN + 2];
  DWORD SessID = wcstol(SR_PathGetArgsW(g_RunData.cmdLine), 0, 10) - 1;
  if (SessID == -1) {
    _tcsncpy(UserName, SR_PathGetArgsW(g_RunData.cmdLine), UNLEN + UNLEN);
    DWORD n = 0;
    WTS_SESSION_INFO *si = 0;
    WTSEnumerateSessions(0, 0, 1, &si, &n);
    if (si && n) {
      for (DWORD i = 0; i < n; i++) {
        TCHAR *un = 0;
        TCHAR *dn = 0;
        DWORD unl = 0;
        DWORD dnl = 0;
        WTSQuerySessionInformation(0, si[i].SessionId, WTSUserName, &un, &unl);
        WTSQuerySessionInformation(0, si[i].SessionId, WTSDomainName, &dn,
                                   &dnl);
        CBigResStr undn(L"%s\\%s", dn, un);
        WTSFreeMemory(dn);
        dn = 0;
        if ((_tcsicmp(un, UserName) == 0) || (_tcsicmp(undn, UserName) == 0)) {
          WTSFreeMemory(un);
          SessID = si[i].SessionId;
          break;
        }
        WTSFreeMemory(un);
        un = 0;
      }
      WTSFreeMemory(si);
    }
    if (SessID == -1) {
      SafeMsgBox(0, CBigResStr(IDS_NOTLOGGEDON, UserName), CResStr(IDS_APPNAME),
                 MB_ICONINFORMATION | MB_SERVICE_NOTIFICATION);
      return RETVAL_CANCELLED;
    }
  }
  {
    HANDLE tSess = GetSessionUserToken(SessID);
    if (!tSess) {
      SafeMsgBox(0, CBigResStr(IDS_NOTLOGGEDON2, SessID), CResStr(IDS_APPNAME),
                 MB_ICONINFORMATION | MB_SERVICE_NOTIFICATION);
      return RETVAL_CANCELLED;
    }
    GetTokenUserName(tSess, UserName);
    CloseHandle(tSess);
    if (!UserName[0]) {
      SafeMsgBox(0, CBigResStr(IDS_NOTLOGGEDON2, SessID), CResStr(IDS_APPNAME),
                 MB_ICONINFORMATION | MB_SERVICE_NOTIFICATION);
      return RETVAL_CANCELLED;
    }
  }
  if (SessID == g_RunData.SessionID)
    return RETVAL_OK;
  if (!SavedPasswordOk(SessID, g_RunData.UserName, UserName)) {
    if (!CreateSafeDesktop(g_RunData.WinSta, g_RunData.Desk, GetBlurDesk,
                           (!(g_RunData.Groups & IS_TERMINAL_USER)) &
                               GetFadeDesk))
      return RETVAL_CANCELLED;
    if (!ValidateFUSUser(g_RunData.SessionID, g_RunData.UserName, UserName)) {
      DeleteSafeDesktop((!(g_RunData.Groups & IS_TERMINAL_USER)) & GetFadeDesk);
      return RETVAL_CANCELLED;
    }
    DeleteSafeDesktop(false);
  }
  if (SwitchToSession(SessID))
    return RETVAL_OK;
  return RETVAL_ACCESSDENIED;
}

void ShowFUSGUI() {
#ifndef _DEBUG
  if (!CreateSafeDesktop(g_RunData.WinSta, g_RunData.Desk, GetBlurDesk,
                         (!(g_RunData.Groups & IS_TERMINAL_USER)) &
                             GetFadeDesk))
    return;
#endif //_DEBUG
  USERLIST ul;
  ul.Add(g_RunData.UserName);
  {
    DWORD n = 0;
    WTS_SESSION_INFO *si = 0;
    WTSEnumerateSessions(0, 0, 1, &si, &n);
    if (si) {
      for (DWORD i = 0; i < n; i++) {
        TCHAR *un = 0;
        TCHAR *dn = 0;
        DWORD unl = 0;
        DWORD dnl = 0;
        WTSQuerySessionInformation(0, si[i].SessionId, WTSUserName, &un, &unl);
        WTSQuerySessionInformation(0, si[i].SessionId, WTSDomainName, &dn,
                                   &dnl);
        ul.Add(CBigResStr(L"%s\\%s", dn, un));
        WTSFreeMemory(dn);
        WTSFreeMemory(un);
        dn = 0;
        un = 0;
      }
      WTSFreeMemory(si);
    }
  }
#ifdef _DEBUG
  ul.Add(L"Kay");
  ul.Add(L"Administrator");
  ul.Add(L"Nikki");
  ul.Add(L"GastNutzer");
  ul.Add(L"SuperUser");
  ul.Add(L"Kay1");
  ul.Add(L"Administrator1");
  ul.Add(L"Nikki1");
  ul.Add(L"GastNutzer1");
  ul.Add(L"SuperUser2");
  ul.Add(L"Kay2");
  ul.Add(L"Administrator2");
  ul.Add(L"Nikki2");
  ul.Add(L"GastNutzer2");
  ul.Add(L"SuperUser2");
#endif //_DEBUG
  if (ul.GetCount()) {
    ul.LoadUserBitmaps();
    int dx = ul.GetCount();
    int dy = 1;
    if (dx > 32)
      dy = 4;
    else if (dx > 17)
      dy = 3;
    else if (dx > 4)
      dy = 2;
    dx = ul.GetCount() / dy;
    //     int DX=(10+48+10)*dx;
    //     int DY=(10+48+10)*dy;
  }
#ifndef _DEBUG
  DeleteSafeDesktop(false);
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////////////
//
// CheckCliProcess:
//
// checks if rd.CliProcessId is this exe and if rd is g_RunData of the calling
// process equals rd, copies the clients g_RunData to our g_RunData
//////////////////////////////////////////////////////////////////////////////
DWORD CheckCliProcess(RUNDATA &rd) {
  g_CliIsAdmin = FALSE;
  if (rd.CliProcessId == GetCurrentProcessId()) {
    DBGTrace("CheckCliProcess same process as service, exit");
    return 0;
  }
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, rd.CliProcessId);
  if (!hProcess) {
    DBGTrace2("OpenProcess(%d) failed: %s", rd.CliProcessId,
              GetLastErrorNameStatic());
    return 0;
  }
  // g_CliIsAdmin
  {
    HANDLE hTok = NULL;
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, rd.CliThreadId);
    if (hThread) {
      if ((!OpenThreadToken(hThread, TOKEN_DUPLICATE, FALSE, &hTok)) &&
          (GetLastError() != ERROR_NO_TOKEN))
        DBGTrace2("OpenThreadToken(%d) failed: %s", rd.CliThreadId,
                  GetLastErrorNameStatic());
      CloseHandle(hThread);
    }
    if ((!hTok) && (!OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hTok))) {
      DBGTrace2("OpenProcessToken(%d) failed: %s", rd.CliProcessId,
                GetLastErrorNameStatic());
      return CloseHandle(hProcess), 0;
    }
    g_CliIsAdmin = IsAdmin(hTok) != 0;
    CloseHandle(hTok);
  }
  SIZE_T s;
  HMODULE hMod;
  // Check if the calling process is this Executable:
  {
    DWORD d;
    TCHAR f1[MAX_PATH];
    TCHAR f2[MAX_PATH];
    if (!GetProcessFileName(f1, MAX_PATH)) {
      DBGTrace1("GetProcessFileName failed", GetLastErrorNameStatic());
      return CloseHandle(hProcess), 0;
    }
    if (!EnumProcessModules(hProcess, &hMod, sizeof(hMod), &d)) {
      DBGTrace1("EnumProcessModules failed", GetLastErrorNameStatic());
      return CloseHandle(hProcess), 0;
    }
    if (GetModuleFileNameEx(hProcess, hMod, f2, MAX_PATH) == 0) {
      DBGTrace1("GetModuleFileNameEx failed", GetLastErrorNameStatic());
      return CloseHandle(hProcess), 0;
    }
    if (_tcsicmp(f1, f2) != 0) {
      DBGTrace2("Invalid Process! %s != %s !", f1, f2);
      return CloseHandle(hProcess), 0;
    }
  }
  // Since it's the same process, g_RunData has the same address!
  if (!ReadProcessMemory(hProcess, BASE_ADDR(&g_RunData, hProcess), &g_RunData,
                         sizeof(RUNDATA), &s)) {
    DBGTrace1("ReadProcessMemory failed: %s", GetLastErrorNameStatic());
    return CloseHandle(hProcess), 0;
  }
  CloseHandle(hProcess);
  if (sizeof(RUNDATA) != s) {
    DBGTrace2("ReadProcessMemory wrong size. Expected %d, got %d",
              sizeof(RUNDATA), s);
    return 0;
  }
  if (memcmp(&rd, &g_RunData, sizeof(RUNDATA)) != 0) {
    DBGTrace("ReadProcessMemory wrong RunData");
    return 1;
  }
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
//
//  ResumeClient
//
//////////////////////////////////////////////////////////////////////////////
BOOL ResumeClient(int RetVal, bool bWriteRunData = false) {
  HANDLE hProcess =
      OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_RunData.CliProcessId);
  if (!hProcess) {
    DBGTrace1("OpenProcess failed: %s", GetLastErrorNameStatic());
    return FALSE;
  }
  SIZE_T n;
  // Since it's the same process, g_RetVal has the same address!
  if (!WriteProcessMemory(hProcess, BASE_ADDR(&g_RetVal, hProcess), &RetVal,
                          sizeof(int), &n)) {
    DBGTrace1("WriteProcessMemory failed: %s", GetLastErrorNameStatic());
    return CloseHandle(hProcess), FALSE;
  }
  if (sizeof(int) != n)
    DBGTrace2("WriteProcessMemory invalid size %d != %d ", sizeof(int), n);
  if (bWriteRunData) {
    WriteProcessMemory(hProcess, BASE_ADDR(&g_RunData, hProcess), &g_RunData,
                       sizeof(RUNDATA), &n);
    if (sizeof(RUNDATA) != n)
      DBGTrace2("WriteProcessMemory invalid size %d != %d ", sizeof(RUNDATA),
                n);
  }
  CloseHandle(hProcess);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
//  The Service:
//
//////////////////////////////////////////////////////////////////////////////

VOID WINAPI SvcCtrlHndlr(DWORD dwControl) {
  // service control handler
  if (dwControl == SERVICE_CONTROL_STOP) {
    g_ss.dwCurrentState = SERVICE_STOP_PENDING;
    g_ss.dwCheckPoint = 0;
    g_ss.dwWaitHint = 0;
    g_ss.dwWin32ExitCode = 0;
    SetServiceStatus(g_hSS, &g_ss);
    // Close Named Pipe
    HANDLE hPipe = g_hPipe;
    g_hPipe = INVALID_HANDLE_VALUE;
    // Connect to Pipe
    HANDLE sw =
        CreateFile(ServicePipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (sw == INVALID_HANDLE_VALUE) {
      DisconnectNamedPipe(hPipe);
      sw =
          CreateFile(ServicePipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    }
    // As g_hPipe is now INVALID_HANDLE_VALUE, the Service will exit
    CloseHandle(hPipe);
    return;
  }
  if (g_hSS != (SERVICE_STATUS_HANDLE)0)
    SetServiceStatus(g_hSS, &g_ss);
}

//////////////////////////////////////////////////////////////////////////////
//
//  MyCPAU
//
//////////////////////////////////////////////////////////////////////////////

BOOL MyCPAU(HANDLE hToken, LPCTSTR lpApplicationName, LPTSTR lpCommandLine,
            LPSECURITY_ATTRIBUTES lpProcessAttributes,
            LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
            DWORD dwCreationFlags, LPVOID lpEnvironment,
            LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo,
            LPPROCESS_INFORMATION lpProcessInformation) {
  // CreateProcessAsUser will only work from an NT System Account since the
  // Privilege SE_ASSIGNPRIMARYTOKEN_NAME is not present elsewhere
  EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
  EnablePrivilege(SE_INCREASE_QUOTA_NAME);
  BOOL bOK = CreateProcessAsUser(
      hToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
      lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
      lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
  DWORD le = GetLastError();
  // Second try: Admin user
  if ((!bOK) && ((le == ERROR_ACCESS_DENIED) || (le == ERROR_DIRECTORY))) {
    DBGTrace4("WARNING: CreateProcessAsUser(%s,%s,%s) failed: %s; "
              "Impersonating as Admin",
              lpCommandLine, lpEnvironment, lpCurrentDirectory,
              GetErrorNameStatic(le));
    // Impersonation required for EFS and CreateProcessAsUser
    ImpersonateLoggedOnUser(hToken);
    bOK = CreateProcessAsUser(
        hToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    le = GetLastError();
    RevertToSelf();
  }
  // Third try: Session User
  if ((!bOK) && ((le == ERROR_ACCESS_DENIED) || (le == ERROR_DIRECTORY) ||
                 (le == ERROR_INVALID_PASSWORD))) {
    DBGTrace4("WARNING: CreateProcessAsUser(%s,%s,%s) failed: %s; "
              "Impersonating as User",
              lpCommandLine, lpEnvironment, lpCurrentDirectory,
              GetErrorNameStatic(le));
    // Impersonation required for EFS and CreateProcessAsUser
    CImpersonateSessionUser ISU(g_RunData.SessionID);
    bOK = CreateProcessAsUser(
        hToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    le = GetLastError();
  }
  if (!bOK)
    DBGTrace4("CreateProcessAsUser(%s,%s,%s) failed: %s", lpCommandLine,
              lpEnvironment, lpCurrentDirectory, GetErrorNameStatic(le));
  SetLastError(le);
  return bOK;
}

//////////////////////////////////////////////////////////////////////////////
//
//  ShowTrayWarning
//
//////////////////////////////////////////////////////////////////////////////
void ShowTrayWarning(LPCTSTR Text, int IconId, int TimeOut) {
  if ((!g_CliIsInAdmins) && GetHideFromUser(g_RunData.UserName))
    return;
  TCHAR cmd[4096] = {0};
  GetSystemWindowsDirectory(cmd, 4096);
  SR_PathAppendW(cmd, L"SuRun.exe");
  HANDLE hUser = GetSessionUserToken(g_RunData.SessionID);
  PROCESS_INFORMATION pi = {0};
  STARTUPINFO si = {0};
  si.cb = sizeof(si);
  // Do not inherit Desktop from calling process, use Tokens Desktop
  TCHAR WinstaDesk[2 * MAX_PATH + 2];
  _stprintf(WinstaDesk, _T("%s\\%s"), g_RunData.WinSta, g_RunData.Desk);
  si.lpDesktop = WinstaDesk;
  // Show ToolTip "<Program> is running elevated"...
  if (MyCPAU(hUser, NULL, cmd, NULL, NULL, FALSE,
             CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT | DETACHED_PROCESS,
             NULL, NULL, &si, &pi)) {
    // Tell SuRun to Say something:
    RUNDATA rd = g_RunData;
    rd.CliProcessId = 0;
    rd.CliThreadId = pi.dwThreadId;
    rd.RetPtr = 0;
    rd.RetPID = 0;
    rd.IconId = IconId;
    rd.TimeOut = TimeOut;
    _tcscpy(rd.cmdLine, Text);
    DWORD_PTR n = 0;
    if (!WriteProcessMemory(pi.hProcess, BASE_ADDR(&g_RunData, pi.hProcess),
                            &rd, sizeof(RUNDATA), &n))
      TerminateProcess(pi.hProcess, 0);
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
  CloseHandle(hUser);
}

void TestEmptyAdminPasswords() {
  switch (GetAdminNoPassWarn) {
  case APW_ALL:
    break;
  case APW_NR_SR_ADMIN:
    if (g_CliIsInSuRunners && (!GetRestrictApps(g_RunData.UserName)) &&
        (!GetNoRunSetup(g_RunData.UserName)))
      break;
    goto ChkAdmin;
  case APW_SURUN_ADMIN:
    if (g_CliIsInSuRunners)
      break;
  case APW_ADMIN:
  ChkAdmin:
    if (g_CliIsAdmin)
      break;
  case APW_NONE:
  default:
    return;
  }
  USERLIST u;
  u.SetGroupUsers(DOMAIN_ALIAS_RID_ADMINS, g_RunData.SessionID, false);
  TCHAR un[4096] = {0};
  for (int i = 0; i < u.GetCount(); i++)
    if (PasswordOK(g_RunData.SessionID, u.GetUserName(i), 0, TRUE)) {
      DBGTrace1("Warning: %s is an empty password admin", u.GetUserName(i));
      _tcscat(un, u.GetUserName(i));
      _tcscat(un, _T("\n"));
    }
  if (un[0])
    ShowTrayWarning(
        CBigResStr(EmptyPWAllowed ? IDS_EMPTYPASS : IDS_EMPTYPASS2, un),
        IDI_SHIELD2, 0);
}

//////////////////////////////////////////////////////////////////////////////
//
//  ServiceMain
//
//////////////////////////////////////////////////////////////////////////////
BOOL InjectIATHook(HANDLE hProc) {
  HANDLE hThread = 0;
  try {
    PROC pLoadLib =
        GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryA");
    if (!pLoadLib)
      return false;
    char *DllName = "SuRunExt.dll";
    size_t nDll = strlen(DllName) + 1;
    void *RmteName =
        VirtualAllocEx(hProc, NULL, nDll, MEM_COMMIT, PAGE_READWRITE);
    if (RmteName == NULL) {
      DBGTrace1("InjectIATHook VirtualAllocEx failed: %s",
                GetLastErrorNameStatic());
      return false;
    }
    WriteProcessMemory(hProc, RmteName, (void *)DllName, nDll, NULL);
    try {
      hThread = CreateRemoteThread(
          hProc, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLib, RmteName, 0, NULL);
      if (hThread == 0)
        DBGTrace1("InjectIATHook CreateRemoteThread failed: %s",
                  GetLastErrorNameStatic());
    }catch(...) {
      DBGTrace("SuRun: CreateRemoteThread Exeption!");
    }
    if (hThread != NULL) {
      WaitForSingleObject(hThread, INFINITE);
      CloseHandle(hThread);
    }
    VirtualFreeEx(hProc, RmteName, sizeof(DllName), MEM_RELEASE);
  } catch(...) {
    DBGTrace("SuRun: InjectIATHook() Exeption!");
  }
  return hThread != 0;
}

BOOL InjectIATHook(DWORD ProcId) {
  HANDLE hProc =
      OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE |
                      PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION,
                  false, ProcId);
  if (!hProc) {
    DBGTrace2("InjectIATHook OpenProcess(%d) failed: %s", ProcId,
              GetLastErrorNameStatic());
    return false;
  }
  BOOL bRet = InjectIATHook(hProc);
  CloseHandle(hProc);
  return bRet;
}

BOOL InjectIATHook(LPTSTR ProcName) {
  DWORD PID = GetProcessID(ProcName);
  if (PID)
    return InjectIATHook(PID);
  DBGTrace1("InjectIATHook GetProcessID(%s) failed", ProcName);
  return FALSE;
}

void CheckRenamedCoputer() {
  DWORD cnl = 2 * UNLEN;
  TCHAR cn_cur[2 * UNLEN + 1] = {0};
  TCHAR cn_old[2 * UNLEN + 1] = {0};
  if (!GetComputerName(cn_cur, &cnl))
    return;
  if (GetRegStr(HKLM, SVCKEY, L"ComputerName", cn_old, 2 * UNLEN) &&
      (_tcslen(cn_old) > 0)) {
    if ((_tcslen(cn_old) != _tcslen(cn_cur)) || (_tcsicmp(cn_old, cn_cur))) {
      RenameRegKey(HKLM, CBigResStr(SVCKEY _T("\\%s"), cn_old),
                   CBigResStr(SVCKEY _T("\\%s"), cn_cur));
      RenameRegKey(HKCR, CBigResStr(SHEXOPTKEY _T("\\%s"), cn_old),
                   CBigResStr(SHEXOPTKEY _T("\\%s"), cn_cur));
      DelRegKey(HKLM, CBigResStr(SVCKEY _T("\\RunAs\\%s"), cn_old));
    }
  }
  SetRegStr(HKLM, SVCKEY, L"ComputerName", cn_cur);
}

extern TOKEN_STATISTICS g_AdminTStat;
DWORD DirectStartUserProcess(DWORD ProcId); // forward decl
DWORD LSAStartAdminProcess();               // forward decl

static BOOL TestDirectServiceCommands() {
  //
  if (g_RunData.KillPID != 0xFFFFFFFF)
    return false;
  if (_tcsicmp(g_RunData.cmdLine, _T("/TSATHREAD")) == 0) {
    TestEmptyAdminPasswords();
    CloseHandle(CreateThread(0, 0, TSAThreadProc,
                             (VOID *)(DWORD_PTR)g_RunData.CliProcessId, 0, 0));
    return true;
  }
  if (_tcsnicmp(g_RunData.cmdLine, _T("/RESTORE "), 9) == 0) {
    GetProcessUserName(g_RunData.CliProcessId, g_RunData.UserName);
    // Double check if User is Admin!
    if (IsInAdmins(g_RunData.UserName, g_RunData.SessionID) ||
        IsLocalSystem(g_RunData.CliProcessId)) {
      if (!ImportSettings(&g_RunData.cmdLine[9]))
        SafeMsgBox(0, CBigResStr(IDS_IMPORTFAIL, &g_RunData.cmdLine[9]),
                   CResStr(IDS_APPNAME), MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
      ResumeClient(RETVAL_OK);
    } else {
      ResumeClient(RETVAL_ACCESSDENIED);
      SafeMsgBox(0, CBigResStr(IDS_NOIMPORT), CResStr(IDS_APPNAME),
                 MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
    }
    return true;
  }
  if (_tcsnicmp(g_RunData.cmdLine, _T("/BACKUP "), 8) == 0) {
    GetProcessUserName(g_RunData.CliProcessId, g_RunData.UserName);
    // Double check if User is Admin!
    if (IsInAdmins(g_RunData.UserName, g_RunData.SessionID) ||
        IsLocalSystem(g_RunData.CliProcessId)) {
      ExportSettings(&g_RunData.cmdLine[8], g_RunData.SessionID,
                     g_RunData.UserName);
      ResumeClient(RETVAL_OK);
    } else {
      ResumeClient(RETVAL_ACCESSDENIED);
      SafeMsgBox(0, CBigResStr(IDS_NOEXPORT), CResStr(IDS_APPNAME),
                 MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
    }
    return true;
  }
  return false;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
  zero(g_RunPwd);
  // service main
  argc; // unused
  argv; // unused
  DBGTrace("SuRun Service starting");
  g_ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  g_ss.dwCurrentState = SERVICE_START_PENDING;
  g_hSS = RegisterServiceCtrlHandler(GetSvcName(), SvcCtrlHndlr);
  if (g_hSS == (SERVICE_STATUS_HANDLE)0) {
    DBGTrace2("RegisterServiceCtrlHandler(%s) failed: %s", GetSvcName(),
              GetLastErrorNameStatic());
    return;
  }
  // Set Service Status to "running"
  g_ss.dwCurrentState = SERVICE_RUNNING;
  g_ss.dwCheckPoint = 0;
  g_ss.dwWaitHint = 0;
  SetServiceStatus(g_hSS, &g_ss);
  DWORD StartDelay =
      max(0, min(600, int(GetRegInt(HKLM, SURUNKEY, L"StartDelay", 0))));
  // Wait to start the services, because else Windows deletes credentials
  if (StartDelay) {
    DBGTrace1("SuRun Service start delay: %d s", StartDelay);
    Sleep(StartDelay * 1000);
  }
  RestoreUserPasswords();
  CheckRenamedCoputer();
  if (GetUseSVCHook)
    InjectIATHook(L"services.exe");
  // Steal token from LSASS.exe to get SeCreateTokenPrivilege in Vista
  HANDLE hRunLSASS = GetProcessUserToken(GetProcessID(L"lsass.exe"));
  if (hRunLSASS == 0) {
    hRunLSASS = GetProcessUserToken(GetCurrentProcessId());
    DBGTrace("Failed to open LSASS, using own ProcessToken");
  }
  // Create Pipe:
  g_hPipe = CreateNamedPipe(
      ServicePipeName,
      PIPE_ACCESS_INBOUND | WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE,
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 0,
      NMPWAIT_USE_DEFAULT_WAIT, NULL);
  if (g_hPipe != INVALID_HANDLE_VALUE) {
    AllowAccess(g_hPipe);
    DBGTrace("SuRun Service running");
    CTimeOut cto;
    while (g_hPipe != INVALID_HANDLE_VALUE) {
      // Wait for a connection
      ConnectNamedPipe(g_hPipe, 0);
      // Exit if g_hPipe==INVALID_HANDLE_VALUE
      if (g_hPipe == INVALID_HANDLE_VALUE) {
        DBGTrace("SuRun Service g_hPipe==INVALID_HANDLE_VALUE exit");
        break;
      }
      DWORD nRead = 0;
      RUNDATA rd = {0};
      // wait for available data in the pipe
      cto.Set(15000);
      while (PeekNamedPipe(g_hPipe, 0, 0, 0, &nRead, 0) && (!cto.TimedOut()) &&
             (nRead < sizeof(rd)))
        Sleep(2);
      if (nRead >= sizeof(rd))
        // Read Client Process ID and command line
        ReadFile(g_hPipe, &rd, sizeof(rd), &nRead, 0);
      // Disconnect client
      DisconnectNamedPipe(g_hPipe);
      if ((nRead == sizeof(RUNDATA)) && (CheckCliProcess(rd) == 2)) {
        if (TestDirectServiceCommands())
          continue;
        if (!g_RunData.bRunAs) {
          DWORD wlf = GetWhiteListFlags(g_RunData.UserName, g_RunData.cmdLine,
                                        (DWORD)-1);
          bool bNotInList = wlf == -1;
          if (bNotInList)
            wlf = 0;
          // Restricted user that does not launch SuRuns Settings?
          //           TODO("Restricted User can do all by specifying admin
          //           password"); if  (bNotInList &&
          //           GetRestrictApps(g_RunData.UserName) &&
          //           (_tcsicmp(g_RunData.cmdLine,_T("/SETUP"))!=0))
          //           {
          //             ResumeClient(g_RunData.bShlExHook?RETVAL_SX_NOTINLIST:RETVAL_RESTRICT);
          //             DBGTrace2("Restriction WhiteList MisMatch: %s:
          //             %s",g_RunData.UserName,g_RunData.cmdLine) continue;
          //           }
          // check if the requested App is Flagged with AutoCancel
          if ((wlf & (FLAG_AUTOCANCEL | FLAG_SHELLEXEC)) == FLAG_AUTOCANCEL) {
            ResumeClient((g_RunData.bShlExHook) ? RETVAL_SX_NOTINLIST
                                                : RETVAL_CANCELLED);
            DBGTrace2("ShellExecute AutoCancel WhiteList MATCH: %s: %s",
                      g_RunData.UserName, g_RunData.cmdLine);
            continue;
          }
          if ((wlf & (FLAG_AUTOCANCEL | FLAG_SHELLEXEC)) ==
              (FLAG_AUTOCANCEL | FLAG_SHELLEXEC)) {
            DBGTrace2("ShellExecute AutoRunLOW WhiteList MATCH: %s: %s",
                      g_RunData.UserName, g_RunData.cmdLine);
            ResumeClient(DirectStartUserProcess(0), true);
            continue;
          }
          // check if the requested App is in the ShellExecHook-Runlist
          if (g_RunData.bShlExHook) {
            // check if the requested App is Flagged with AutoCancel
            if (wlf & FLAG_CANCEL_SX) {
              ResumeClient(RETVAL_SX_NOTINLIST);
              DBGTrace2("ShellExecute AutoCancel WhiteList MATCH: %s: %s",
                        g_RunData.UserName, g_RunData.cmdLine);
              continue;
            }
            // Only SuRunners can use the hooks
            if (!g_CliIsInSuRunners) {
              ResumeClient(RETVAL_SX_NOTINLIST);
              DBGTrace2("User is no SuRunner: %s: %s", g_RunData.UserName,
                        g_RunData.cmdLine);
              continue;
            }
            if (!(wlf & FLAG_SHELLEXEC)) {
              // check for requireAdministrator Manifest and
              // file names *setup*;*install*;*update*;*.msi;*.msc
              if (!RequiresAdmin(g_RunData.cmdLine)) {
                ResumeClient(RETVAL_SX_NOTINLIST);
                // DBGTrace2("ShellExecute WhiteList MisMatch: %s:
                // %s",g_RunData.UserName,g_RunData.cmdLine)
                continue;
              }
            }
            // DBGTrace2("ShellExecute WhiteList Match: %s:
            // %s",g_RunData.UserName,g_RunData.cmdLine)
          }
        } else // if (!g_RunData.bRunAs)
        {
          if ((rd.bRunAs & 3) == 2) // LOW option:
          {
            ResumeClient(DirectStartUserProcess(0), true);
            continue;
          }
          if (rd.bRunAs & 4) // Username given
          {
            if (_tcsicmp(rd.UserName, _T("SYSTEM")) == 0) {
              GetProcessUserName(g_RunData.CliProcessId, rd.UserName);
              HANDLE hUser = GetProcessUserToken(g_RunData.CliProcessId);
              DWORD IsIn = UserIsInSuRunnersOrAdmins(hUser);
              CloseHandle(hUser);
              // Only real Admins or real SuRunners may run as SYSTEM
              if (((IsIn & IS_IN_ADMINS) == 0) &&
                  (((IsIn & IS_IN_SURUNNERS) == 0) ||
                   (GetRestrictApps(g_RunData.UserName)))) {
                ResumeClient(RETVAL_ACCESSDENIED);
                continue;
              }
            } else if (_tcschr(rd.UserName, L'\\') == NULL) {

              DWORD n = sizeof(g_RunData.UserName);
              GetComputerName(g_RunData.UserName, &n);
              PathAppend(g_RunData.UserName, rd.UserName);
            }
            GetProcessUserName(g_RunData.CliProcessId, rd.UserName);
            SetRegStr(HKLM, USERKEY(rd.UserName), L"LastRunAsUser",
                      g_RunData.UserName);
            g_RunData = rd;
          }
        }
        // Process Check succeeded, now start this exe in the calling processes
        // Terminal server session to get SwitchDesktop working:
        if (hRunLSASS) {
          DWORD SessionID = 0;
          ProcessIdToSessionId(g_RunData.CliProcessId, &SessionID);
          if (SetTokenInformation(hRunLSASS, TokenSessionId, &SessionID,
                                  sizeof(DWORD))) {
            TCHAR WinstaDesk[2 * MAX_PATH + 2];
            _stprintf(WinstaDesk, _T("%s\\%s"), g_RunData.WinSta,
                      g_RunData.Desk);
            STARTUPINFO si = {0};
            si.cb = sizeof(si);
            si.lpDesktop = WinstaDesk;
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;
            TCHAR cmd[4096] = {0};
            GetSystemWindowsDirectory(cmd, 4096);
            SR_PathAppendW(cmd, L"SuRun.exe");
            SR_PathQuoteSpacesW(cmd);
            _tcscat(cmd, L" /AskUSER");
            EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
            EnablePrivilege(SE_INCREASE_QUOTA_NAME);
            BYTE bRunCount = 0;
          TryAgain:
            PROCESS_INFORMATION pi = {0};
            DWORD stTime = timeGetTime();
            if (CreateProcessAsUser(hRunLSASS, NULL, cmd, NULL, NULL, FALSE,
                                    CREATE_SUSPENDED |
                                        CREATE_UNICODE_ENVIRONMENT |
                                        HIGH_PRIORITY_CLASS,
                                    0, NULL, &si, &pi)) {
              bRunCount++;
              SIZE_T N = 0;
              if (!WriteProcessMemory(pi.hProcess,
                                      BASE_ADDR(&g_RunData, pi.hProcess), &rd,
                                      sizeof(rd), &N))
                DBGTrace1("WriteProcessMemory failed %s",
                          GetLastErrorNameStatic());

              if (!WriteProcessMemory(pi.hProcess,
                                      BASE_ADDR(&g_CliIsAdmin, pi.hProcess),
                                      &g_CliIsAdmin, sizeof(g_CliIsAdmin), &N))
                DBGTrace1("WriteProcessMemory failed %s",
                          GetLastErrorNameStatic());
              // if (!g_RunData.bRunAs)
              {
                HANDLE hTok = 0;
                if (g_RunData.Groups & IS_IN_ADMINS)
                  hTok = GetProcessUserToken(g_RunData.CliProcessId);
                else
                  hTok = GetTempAdminToken(g_RunData.UserName);
                TOKEN_STATISTICS tstat = {0};
                if (hTok) {
                  DWORD n = 0;
                  CHK_BOOL_FN(GetTokenInformation(hTok, TokenStatistics, &tstat,
                                                  sizeof(tstat), &n));
                  if (g_RunData.Groups & IS_IN_ADMINS)
                    CloseHandle(hTok);
                } else
                  DBGTrace("GetProcessUserToken NOT ok");
                if (!WriteProcessMemory(pi.hProcess,
                                        BASE_ADDR(&g_AdminTStat, pi.hProcess),
                                        &tstat, sizeof(TOKEN_STATISTICS), &N))
                  DBGTrace1("WriteProcessMemory failed %s",
                            GetLastErrorNameStatic());
              }
              ResumeThread(pi.hThread);
              CloseHandle(pi.hThread);
              WaitForSingleObject(pi.hProcess, INFINITE);
              DWORD ex = 0;
              GetExitCodeProcess(pi.hProcess, &ex);
              CloseHandle(pi.hProcess);
              if (ex != (0x0FFFFFFF & (~pi.dwProcessId))) {
                DBGTrace4(
                    "SuRun: Starting child process try %d failed after %d ms. "
                    L"Expected return value :%X real return value %x",
                    bRunCount, timeGetTime() - stTime,
                    0x0FFFFFFF & (~pi.dwProcessId), ex);
                // For YZshadow: Give it four tries
                if ((bRunCount < 4) && (ex == STATUS_ACCESS_VIOLATION))
                  goto TryAgain;
                ResumeClient(RETVAL_ACCESSDENIED);
              }
            } else
              DBGTrace2("CreateProcessAsUser(%s) failed %s", cmd,
                        GetLastErrorNameStatic());
          } else
            DBGTrace2("SetTokenInformation(TokenSessionId(%d)) failed %s",
                      SessionID, GetLastErrorNameStatic());
        }
      }
      zero(rd);
    }
  } else
    DBGTrace1("CreateNamedPipe failed %s", GetLastErrorNameStatic());
  CloseHandle(hRunLSASS);
  DeleteTempAdminTokens();
  // Stop Service
  g_ss.dwCurrentState = SERVICE_STOPPED;
  g_ss.dwCheckPoint = 0;
  g_ss.dwWaitHint = 0;
  g_ss.dwWin32ExitCode = 0;
  DBGTrace("SuRun Service stopped");
  SetServiceStatus(g_hSS, &g_ss);
}

//////////////////////////////////////////////////////////////////////////////
//
//  KillProcess
//
//////////////////////////////////////////////////////////////////////////////

// callback function for window enumeration
BOOL g_bKilledOne = FALSE;
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
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    g_bKilledOne = TRUE;
  }
  return TRUE;
}

void KillProcess(DWORD PID, DWORD TimeOut = 5000) {
  if (!PID)
    return;
  HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, PID);
  if (!hProcess)
    return;
  // Messages work on the same WinSta/Desk only
  SetProcWinStaDesk(g_RunData.WinSta, g_RunData.Desk);
  // Post WM_CLOSE to all Windows of PID
  g_bKilledOne = FALSE;
  EnumWindows(CloseAppEnum, (LPARAM)PID);
  // Give the Process time to close
  if ((!g_bKilledOne) ||
      (WaitForSingleObject(hProcess, TimeOut) != WAIT_OBJECT_0))
    TerminateProcess(hProcess, 0);
  CloseHandle(hProcess);
}

//////////////////////////////////////////////////////////////////////////////
//
//  BeautifyCmdLine: replace shell namespace object names
//
//////////////////////////////////////////////////////////////////////////////
LPCTSTR GetShellName(LPCTSTR clsid, LPCTSTR DefName) {
  static TCHAR s[MAX_PATH];
  zero(s);
  GetRegStr(HKCR, CResStr(L"CLSID\\%s", clsid), L"LocalizedString", s,
            MAX_PATH);
  if (s[0] == '@') {
    LPTSTR c = _tcsrchr(s, ',');
    if (c) {
      *c = 0;
      c++;
      int i = _ttoi(c);
      HINSTANCE h = LoadLibrary(&s[1]);
      LoadString(h, (DWORD)-i, s, MAX_PATH);
      FreeLibrary(h);
    }
  }
  if (!s[0])
    GetRegStr(HKCR, CResStr(L"CLSID\\%s", clsid), L"", s, MAX_PATH);
  if (!s[0])
    _tcscpy(s, DefName);
  return s;
}

LPCTSTR BeautifyCmdLine(LPTSTR cmd) {
  typedef struct {
    int nId;
    LPCTSTR clsid;
  } ShName;
  ShName shn[] = {
      {IDS_SHNAME1,
       _T("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}")}, //"My Computer"
      {IDS_SHNAME2,
       _T("::{208D2C60-3AEA-1069-A2D7-08002B30309D}")}, //"My Network Places"
      {IDS_SHNAME3,
       _T("::{7007ACC7-3202-11D1-AAD2-00805FC1270E}")}, //"Network connections"
      {IDS_SHNAME4,
       _T("::{21EC2020-3AEA-1069-A2DD-08002B30309D}")}, //"Control Panel"
      {IDS_SHNAME4,
       _T("::{26EE0668-A00A-44D7-9371-BEB064C98683}")}, //"Control Panel"
      {IDS_SHNAME5,
       _T("::{645FF040-5081-101B-9F08-00AA002F954E}")}, //"Recycle Bin"
      {IDS_SHNAME6, _T("::{D20EA4E1-3957-11d2-A40B-0C5020524152}")}, //"Fonts"
      {IDS_SHNAME7,
       _T("::{D20EA4E1-3957-11d2-A40B-0C5020524153}")}, //"Administrative Tools"
      {IDS_SHNAME8,
       _T("::{450D8FBA-AD25-11D0-98A8-0800361B1103}")} //"My Documents"
  };
  static TCHAR c1[4096];
  zero(c1);
  _tcscpy(c1, cmd);
  bool bOk = false;
  int i = 0;
  for (i = 0; i < countof(shn); i++) {
    LPTSTR c = _tcsstr(c1, shn[i].clsid);
    if (c) {
      bOk = TRUE;
      *c = 0;
      TCHAR c2[4096];
      _stprintf(c2, L"%s%s%s", c1,
                GetShellName(&shn[i].clsid[2], CResStr(shn[i].nId)),
                &c[_tcslen(shn[i].clsid)]);
      _tcscpy(c1, c2);
    }
  }
  if (bOk)
    _tcscpy(c1, CBigResStr(IDS_BEAUTIFIED, cmd, c1));
  return c1;
}

//////////////////////////////////////////////////////////////////////////////
//
//  PrepareSuRun: Show Password/Permission Dialog on secure Desktop,
//
//////////////////////////////////////////////////////////////////////////////
DWORD PrepareSuRun() {
  // #ifdef DoDBGTrace
  //   AddTime("PrepareSuRun start")
  // #endif //DoDBGTrace
  zero(g_RunPwd);
  if ((!g_CliIsInSuRunners) && GetNoConvUser) {
    DBGTrace1("PrepareSuRun EXIT: User %s is no SuRunner, no auto convert",
              g_RunData.UserName);
    return RETVAL_ACCESSDENIED;
  }
  BOOL PwExpired = PasswordExpired(g_RunData.UserName);
  BOOL NoNeedPw = g_AdminTStat.AuthenticationId.HighPart ||
                  g_AdminTStat.AuthenticationId.LowPart;
  // Ask For Password?
  BOOL PwOk = GetSavePW && (!PwExpired) && NoNeedPw;
  DWORD f = GetWhiteListFlags(g_RunData.UserName, g_RunData.cmdLine, (DWORD)-1);
  bool bNotInList = f == -1;
  if (bNotInList)
    f = 0;
  if (!PwOk) {
    // Delete last password "ok" time
    DeletePassword(g_RunData.UserName);
    // if "Never ask for a password", just run the program
    if (NoNeedPw && ((f & (FLAG_DONTASK | FLAG_NEVERASK)) ==
                     (FLAG_DONTASK | FLAG_NEVERASK)))
      return RETVAL_OK;
  } else if (f & (FLAG_DONTASK | FLAG_NEVERASK)) {
    UpdLastRunTime(g_RunData.UserName);
    return RETVAL_OK;
  }
  if (g_RunData.bShExNoSafeDesk) {
    DBGTrace1("PrepareSuRun(%s) EXIT: bShExNoSafeDesk", g_RunData.cmdLine);
    return RETVAL_SX_NOTINLIST;
  }
  // Get real groups for the user: (Not just the groups from the Client Token)
  // #ifdef DoDBGTrace
  //   AddTime("IsInSuRunnersOrAdmins start")
  // #endif //DoDBGTrace
  g_RunData.Groups =
      IsInSuRunnersOrAdmins(g_RunData.UserName, g_RunData.SessionID);
  if (HideSuRun(g_RunData.UserName, g_RunData.Groups)) {
    DBGTrace1("PrepareSuRun EXIT: SuRun is hidden for User %s",
              g_RunData.UserName);
    return RETVAL_CANCELLED;
  }
  // Create the safe desktop
  bool bFadeDesk = (!(g_RunData.Groups & IS_TERMINAL_USER)) && GetFadeDesk;
  // #ifdef DoDBGTrace
  //   AddTime("CreateSafeDesktop start")
  // #endif //DoDBGTrace
  if (!CreateSafeDesktop(g_RunData.WinSta, g_RunData.Desk, GetBlurDesk,
                         bFadeDesk)) {
    DBGTrace("PrepareSuRun EXIT: CreateSafeDesktop failed");
    return (DWORD)RETVAL_NODESKTOP;
  }
  // #ifdef DoDBGTrace
  //   AddTime("CreateSafeDesktop done")
  // #endif //DoDBGTrace
  try {
    // safe desktop created...
    if ((!g_CliIsInSuRunners) &&
        (!BecomeSuRunner(g_RunData.UserName, g_RunData.SessionID,
                         g_CliIsInAdmins, g_CliIsSplitAdmin, TRUE, 0))) {
      DeleteSafeDesktop(bFadeDesk);
      return RETVAL_CANCELLED;
    }
    if (!g_CliIsInSuRunners) {
      PwOk = GetSavePW && (!PasswordExpired(g_RunData.UserName));
      g_RunData.Groups |= IS_IN_SURUNNERS;
    }
    // Testing if User is restricted has already been done in Service Main
    DWORD l = 0;
    // if Shell executes unknown app ask for AutoMagic, else ask for permission
    int IDSMsg = (g_RunData.bShlExHook && ((f & FLAG_SHELLEXEC) == 0))
                     ? IDS_ASKAUTO
                     : IDS_ASKOK;
    if (!PwOk) {
      if (f & FLAG_DONTASK) // Program is known but password expired: Only check
                            // password!
      {
        bool bOK = ValidateCurrentUser(g_RunData.SessionID, g_RunData.UserName,
                                       g_RunPwd, IDS_ASKOK,
                                       BeautifyCmdLine(g_RunData.cmdLine)) &
                   1;
        DeleteSafeDesktop((!bOK) && bFadeDesk);
        if (!bOK)
          return RETVAL_CANCELLED;
        UpdLastRunTime(g_RunData.UserName);
        if (!NoNeedPw) // Only if we don't have the admin Token:
        {
          // Save the password for user in ServiceMain:
          SavePassword(g_RunData.UserName, g_RunPwd);
          HANDLE hUser = LogonAsAdmin(g_RunData.UserName, g_RunPwd);
          DWORD n = 0;
          if (hUser)
            CHK_BOOL_FN(GetTokenInformation(hUser, TokenStatistics,
                                            &g_AdminTStat, sizeof(g_AdminTStat),
                                            &n));
          zero(g_RunPwd);
          // Do not call CloseHandle(hUser)!
        }
        return RETVAL_OK;
      }
      // Restricted user, unknown app, password not ok
      if ((!NoNeedPw) && GetRestrictApps(g_RunData.UserName)) {
        // Verify Admin, then Logon user and save password
        l = ValidateAdmin(g_RunData.SessionID, IDSMsg,
                          BeautifyCmdLine(g_RunData.cmdLine));
        if ((l & 1) == 0)
          return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST : RETVAL_RESTRICT;
        SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine, FLAG_SHELLEXEC,
                         (l & 4) != 0);
        l = ValidateCurrentUser(g_RunData.SessionID, g_RunData.UserName,
                                g_RunPwd, IDS_ENTER_PW);
      } else
        l = LogonCurrentUser(g_RunData.SessionID, g_RunData.UserName, g_RunPwd,
                             f, IDSMsg, BeautifyCmdLine(g_RunData.cmdLine));
      if ((!NoNeedPw) && (l & 1)) // Only if we don't have the admin Token:
      {
        PwOk = TRUE;
        // Save the password for user in ServiceMain:
        SavePassword(g_RunData.UserName, g_RunPwd);
        HANDLE hUser = LogonAsAdmin(g_RunData.UserName, g_RunPwd);
        DWORD n = 0;
        if (hUser)
          CHK_BOOL_FN(GetTokenInformation(hUser, TokenStatistics, &g_AdminTStat,
                                          sizeof(g_AdminTStat), &n));
        zero(g_RunPwd);
        // Do not call CloseHandle(hUser)!
      }
    } else //(!PwOk):
    {
      // Restricted user, password ok, unknown app
      if (bNotInList && GetRestrictApps(g_RunData.UserName)) {
        // Verify Admin, then Logon user and save password
        l = ValidateAdmin(g_RunData.SessionID, IDSMsg,
                          BeautifyCmdLine(g_RunData.cmdLine));
        if ((l & 1) == 0)
          return g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST : RETVAL_RESTRICT;
      } else
        l = AskCurrentUserOk(g_RunData.SessionID, g_RunData.UserName, f, IDSMsg,
                             BeautifyCmdLine(g_RunData.cmdLine));
    }
    DeleteSafeDesktop(bFadeDesk && ((l & 1) == 0));
    // Cancel:
    if ((l & 1) == 0) {
      if (PwOk && (!GetNoRunSetup(g_RunData.UserName))) {
        if (g_RunData.bShlExHook) {
          // ShellExecHook:
          SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine,
                           FLAG_CANCEL_SX, (l & 2) != 0);
          if ((l & 2) != 0)
            SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine,
                             FLAG_SHELLEXEC, 0);
        } else {
          // SuRun cmdline:
          SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine,
                           FLAG_AUTOCANCEL, (l & 2) != 0);
          if ((l & 2) != 0)
            SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine,
                             FLAG_DONTASK | FLAG_NEVERASK, 0);
        }
      }
      return RETVAL_CANCELLED;
    }
    // Ok:
    if (!GetNoRunSetup(g_RunData.UserName)) {
      SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine, FLAG_DONTASK,
                       (l & 2) != 0);
      if ((l & 2) != 0)
        SetWhiteListFlag(
            g_RunData.UserName, g_RunData.cmdLine,
            g_RunData.bShlExHook ? FLAG_CANCEL_SX : FLAG_AUTOCANCEL, 0);
      else
        SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine, FLAG_NEVERASK,
                         0);
      SetWhiteListFlag(g_RunData.UserName, g_RunData.cmdLine, FLAG_SHELLEXEC,
                       (l & 4) != 0);
    }
    UpdLastRunTime(g_RunData.UserName);
    return RETVAL_OK;
  } catch(...) {
    DBGTrace("FATAL: Exception in PrepareSuRun()");
    DeleteSafeDesktop(false);
    return RETVAL_CANCELLED;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//  CheckServiceStatus
//
//////////////////////////////////////////////////////////////////////////////

DWORD CheckServiceStatus(LPCTSTR ServiceName = GetSvcName()) {
  SC_HANDLE hdlSCM =
      OpenSCManager(0, 0, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
  if (hdlSCM == 0) {
    DBGTrace1("OpenSCManager failed: %s", GetLastErrorNameStatic());
    return FALSE;
  }
  SC_HANDLE hdlServ = OpenService(hdlSCM, ServiceName, SERVICE_QUERY_STATUS);
  if (!hdlServ) {
    DBGTrace1("OpenService failed: %s", GetLastErrorNameStatic());
    return CloseServiceHandle(hdlSCM), FALSE;
  }
  SERVICE_STATUS ss = {0};
  if (!QueryServiceStatus(hdlServ, &ss)) {
    DBGTrace1("QueryServiceStatus failed: %s", GetLastErrorNameStatic());
    CloseServiceHandle(hdlServ);
    return CloseServiceHandle(hdlSCM), FALSE;
  }
  CloseServiceHandle(hdlServ);
  CloseServiceHandle(hdlSCM);
  return ss.dwCurrentState;
}

//////////////////////////////////////////////////////////////////////////////
//
//  Setup
//
//////////////////////////////////////////////////////////////////////////////

BOOL Setup() {
  // check if user name may not run setup:
  if (GetNoRunSetup(g_RunData.UserName)) {
    if (!LogonAdmin(g_RunData.SessionID, IDS_NOADMIN2, g_RunData.UserName))
      return FALSE;
    return RunSetup(g_RunData.SessionID, g_RunData.UserName);
  }
  // check if user name needs to enter the password:
  if (GetReqPw4Setup(g_RunData.UserName)) {
    if (!ValidateCurrentUser(g_RunData.SessionID, g_RunData.UserName, g_RunPwd,
                             IDS_PW4SETUP))
      return FALSE;
    UpdLastRunTime(g_RunData.UserName);
    zero(g_RunPwd);
    return RunSetup(g_RunData.SessionID, g_RunData.UserName);
  }
  // only Admins and SuRunners may setup SuRun
  if (g_CliIsAdmin)
    return RunSetup(g_RunData.SessionID, g_RunData.UserName);
  // If no users should become SuRunners, ask for Admin credentials
  g_RunData.Groups =
      IsInSuRunnersOrAdmins(g_RunData.UserName, g_RunData.SessionID);
  if ((!g_CliIsInSuRunners) && GetNoConvUser) {
    if (!LogonAdmin(g_RunData.SessionID, IDS_NOADMIN2, g_RunData.UserName))
      return FALSE;
    return RunSetup(g_RunData.SessionID, g_RunData.UserName);
  }
  if (g_CliIsInSuRunners ||
      BecomeSuRunner(g_RunData.UserName, g_RunData.SessionID, g_CliIsInAdmins,
                     g_CliIsSplitAdmin, TRUE, 0)) {
    if ((!GetSavePW) || PasswordExpired(g_RunData.UserName)) {
      if (!ValidateCurrentUser(g_RunData.SessionID, g_RunData.UserName,
                               g_RunPwd, IDS_PW4SETUP))
        return FALSE;
      UpdLastRunTime(g_RunData.UserName);
    }
    zero(g_RunPwd);
    return RunSetup(g_RunData.SessionID, g_RunData.UserName);
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
//
//  LSAStartAdminProcess
//
//////////////////////////////////////////////////////////////////////////////
HANDLE GetUserToken(DWORD SessionID, LPCTSTR UserName, LPTSTR Password,
                    bool bRunAs, bool bNoAdmin) {
  // Admin Token for SessionId
  if (bRunAs) // different user, password
  {
    TCHAR un[2 * UNLEN + 2] = {0};
    TCHAR dn[2 * UNLEN + 2] = {0};
    _tcscpy(un, UserName);
    PathStripPath(un);
    _tcscpy(dn, UserName);
    PathRemoveFileSpec(dn);
    return LSALogon(SessionID, un, dn, Password, bNoAdmin);
  }
  if (bNoAdmin) // same user, no admin
    return GetSessionUserToken(SessionID);
  // same user admin
  return GetAdminToken(SessionID);
}

#define GetSeparateProcess(k)                                                  \
  GetRegInt(                                                                   \
      k,                                                                       \
      _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),  \
      _T("SeparateProcess"), 0)
#define SetSeparateProcess(k, b)                                               \
  SetRegInt(                                                                   \
      k,                                                                       \
      _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),  \
      _T("SeparateProcess"), b)

#define DESKTOPPROXYCLASS TEXT("Proxy Desktop")

BOOL CALLBACK KillProxyDesktopEnum(HWND hwnd, LPARAM lParam) {
  TCHAR cn[MAX_PATH];
  GetClassName(hwnd, cn, countof(cn));
  if (_tcsicmp(cn, DESKTOPPROXYCLASS) == 0) {
    if (lParam) {
      DWORD pid = 0;
      GetWindowThreadProcessId(hwnd, &pid);
      if (pid == *((DWORD *)lParam)) {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        *((DWORD *)lParam) = 0;
        return FALSE;
      }
    } else
      SendMessage(hwnd, WM_CLOSE, 0, 0);
  }
  return TRUE;
}

#define W7ExplCOMkey L"AppID\\{CDCBCFCA-3CDC-436f-A4E2-0E02075250C2}"

DWORD LSAStartAdminProcess() {
  DWORD RetVal = RETVAL_ACCESSDENIED;
  // Get Admin User Token and Job object token
  HANDLE hAdmin =
      GetUserToken(g_RunData.SessionID, g_RunData.UserName, g_RunPwd,
                   (g_RunData.bRunAs & 1) != 0, (g_RunData.bRunAs & 2) != 0);
  // Clear Password
  zero(g_RunPwd);
  if (!hAdmin) {
    DBGTrace("FATAL: Could not create user token!");
    return RetVal;
  }
  SetTokenInformation(hAdmin, TokenSessionId, &g_RunData.SessionID,
                      sizeof(DWORD));
  //   if (g_RunData.ConsolePID)
  //   {
  //     HANDLE
  //     hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,g_RunData.ConsolePID); if
  //     (hProcess && SetProcessUserToken(hProcess,hAdmin))
  //       RetVal=RETVAL_OK;
  //     OpenThread()
  //     CloseHandle(hProcess);
  //     CloseHandle(hAdmin);
  //     return RetVal;
  //   }
  PROCESS_INFORMATION pi = {0};
  PROFILEINFO ProfInf = {
      sizeof(ProfInf), PI_NOUI, g_RunData.UserName, 0, 0, 0, 0, 0};
  if (LoadUserProfile(hAdmin, &ProfInf)) {
    void *Env = 0;
    if (CreateEnvironmentBlock(&Env, hAdmin, FALSE)) {
      STARTUPINFO si = {0};
      si.cb = sizeof(si);
      // Do not inherit Desktop from calling process, use Tokens Desktop
      TCHAR WinstaDesk[2 * MAX_PATH + 2];
      _stprintf(WinstaDesk, _T("%s\\%s"), g_RunData.WinSta, g_RunData.Desk);
      si.lpDesktop = WinstaDesk;
      // Special handling for Explorer:
      BOOL bIsExplorer = FALSE;
      {
        TCHAR app[MAX_PATH] = {0};
        TCHAR cmd[4096] = {0};
        _tcscpy(cmd, g_RunData.cmdLine);
        PathRemoveArgs(cmd);
        PathUnquoteSpaces(cmd);
        // Explorer?
        GetSystemWindowsDirectory(app, MAX_PATH);
        PathAppend(app, _T("explorer.exe"));
        bIsExplorer = _tcsicmp(cmd, app) == 0;
        if (bIsExplorer &&
            (_winmajor >=
             6)) // Vista and newer, use "/separate" command line option
        {
          PathQuoteSpaces(cmd);
          _tcscat(cmd, _T(" /SEPARATE, "));
          _tcscat(cmd, PathGetArgs(g_RunData.cmdLine));
          _tcscpy(g_RunData.cmdLine, cmd);
        }
        if (!bIsExplorer) {
          // Cmd?
          GetSystemWindowsDirectory(app, MAX_PATH);
          PathAppend(app, _T("system32\\cmd.exe"));
          if (_tcsicmp(cmd, app) == 0) {
            // cmd.exe: Disable AutoRuns
            PathQuoteSpaces(cmd);
            _tcscat(cmd, _T(" /D "));
            _tcscat(cmd, PathGetArgs(g_RunData.cmdLine));
            _tcscpy(g_RunData.cmdLine, cmd);
          }
        }
      }
      if (MyCPAU(hAdmin, NULL, g_RunData.cmdLine, NULL, NULL, FALSE,
                 CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, Env,
                 g_RunData.CurDir, &si, &pi)) {
        // DBGTrace1("CreateProcessAsUser(%s) OK",g_RunData.cmdLine);
        if (_winmajor >= 6) {
          // Vista: Set SYNCHRONIZE only access for Logon SID
          HANDLE hShell = GetSessionUserToken(g_RunData.SessionID);
          PSID ShellSID = GetLogonSid(hShell);
          SetAdminDenyUserAccess(pi.hProcess, ShellSID,
                                 SURUN_PROCESS_ACCESS_FLAGS);
          SetAdminDenyUserAccess(pi.hThread, ShellSID,
                                 SURUN_THREAD_ACCESS_FLAGS);
          free(ShellSID);
          CloseHandle(hShell);
        }
        // Special handling for Explorer:
        BOOL orgSP = 1;
        PACL pOldDACL = NULL;
        if (bIsExplorer) {
          // Before Vista: kill Desktop Proxy
          if (_winmajor < 6) {
            // To start control Panel and other Explorer children we need to
            // tell Explorer to open folders in a new process
            orgSP = GetSeparateProcess((HKEY)ProfInf.hProfile);
            if (!orgSP)
              SetSeparateProcess((HKEY)ProfInf.hProfile, 1);
            // Messages work on the same WinSta/Desk only
            SetProcWinStaDesk(g_RunData.WinSta, g_RunData.Desk);
            // call DestroyWindow() for each "Desktop Proxy" Windows Class in an
            // Explorer.exe, this will cause a new Explorer.exe to stay running
            EnumWindows(KillProxyDesktopEnum, 0);
          } else if (
              IsWin7pp) // Win7: Set
                        // HKCR\AppID\{CDCBCFCA-3CDC-436f-A4E2-0E02075250C2}\RunAs:
          {
            pOldDACL = RegGrantAdminAccess(HKCR, W7ExplCOMkey);
            // Rename
            if (!RegRenameVal(HKCR, W7ExplCOMkey, L"RunAs", L"_RunAs"))
              DBGTrace("RegRenameVal error!");
          }
        }
        // SetParentProcessID(pi.hProcess,GetParentProcessID(g_RunData.CliProcessId));
        // ToDo: Hooks must call ResumeThread, not Service!
        // but... limited user has no PROCESS_SUSPEND_RESUME or
        // THREAD_SUSPEND_RESUME access
        ResumeThread(pi.hThread);
        if (bIsExplorer) {
          // Before Vista: wait for and kill Desktop Proxy
          if (_winmajor < 6) {
            // Messages work on the same WinSta/Desk only
            SetProcWinStaDesk(g_RunData.WinSta, g_RunData.Desk);
            // call DestroyWindow() for each "Desktop Proxy" Windows Class in an
            // Explorer.exe, this will cause a new Explorer.exe to stay running
            CTimeOut to(10000);
            DWORD pid = pi.dwProcessId;
            while ((!to.TimedOut()) && pid &&
                   EnumWindows(KillProxyDesktopEnum, (LPARAM)&pid) &&
                   (WaitForSingleObject(pi.hProcess, 100) == WAIT_TIMEOUT))
              ;
            if (!orgSP)
              SetSeparateProcess((HKEY)ProfInf.hProfile, 0);
          } else if (IsWin7pp) // Restore original RunAs:
          {
            WaitForSingleObject(pi.hProcess, 10000);
            if (!RegRenameVal(HKCR,
                              L"AppID\\{CDCBCFCA-3CDC-436f-A4E2-0E02075250C2}",
                              L"_RunAs", L"RunAs"))
              DBGTrace("RegRenameVal error!");
            if (pOldDACL)
              RegSetDACL(HKCR, W7ExplCOMkey, pOldDACL);
          }
        }
        if (pOldDACL)
          free(pOldDACL);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        g_RunData.NewPID = pi.dwProcessId;
        RetVal = RETVAL_OK;
        // ShellExec-Hook: We must return the PID and TID to fake CreateProcess:
        if ((g_RunData.RetPID) && (g_RunData.RetPtr)) {
          HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                                        FALSE, g_RunData.RetPID);
          if (hProcess) {
            RET_PROCESS_INFORMATION rpi;
            rpi.dwProcessId = pi.dwProcessId;
            rpi.dwThreadId = pi.dwThreadId;
            SIZE_T n;
            if (!WriteProcessMemory(hProcess, (LPVOID)g_RunData.RetPtr, &rpi,
                                    sizeof(rpi), &n))
              DBGTrace4(
                  "SuRun(%s) WriteProcessMemory to PID %d at %p failed: %s",
                  g_RunData.cmdLine, g_RunData.RetPID, g_RunData.RetPtr,
                  GetLastErrorNameStatic());
            CloseHandle(hProcess);
          } else
            DBGTrace2("AutoSuRun(%s) OpenProcess failed: %s", g_RunData.cmdLine,
                      GetLastErrorNameStatic());
        }
        if (g_RunData.bShlExHook &&
            ((GetCommonWhiteListFlags(g_RunData.UserName, g_RunData.cmdLine,
                                      0) &
              FLAG_SHELLEXEC) == 0))
          ShowTrayWarning(
              CBigResStr(IDS_STARTED, BeautifyCmdLine(g_RunData.cmdLine)),
              IDI_SHIELD0, GetTrayTimeOut * 1000);
      } else
        DBGTrace1("CreateProcessAsUser failed: %s", GetLastErrorNameStatic());
      DestroyEnvironmentBlock(Env);
    } else
      DBGTrace1("CreateEnvironmentBlock failed: %s", GetLastErrorNameStatic());
    if (((g_RunData.bRunAs & 1) == 0) || (RetVal != RETVAL_OK))
      UnloadUserProfile(hAdmin, ProfInf.hProfile);
  } else
    DBGTrace1("LoadUserProfile failed: %s", GetLastErrorNameStatic());
  CloseHandle(hAdmin);
  return RetVal;
}

DWORD DirectStartUserProcess(DWORD ProcId) {
  DWORD RetVal = RETVAL_ACCESSDENIED;
  // ProcId==0; Start process with shell token
  HANDLE hUser = ProcId ? GetProcessUserToken(ProcId)
                        : GetSessionUserToken(g_RunData.SessionID);
  { // Start in same session as Client:
    DWORD SessionID = 0;
    ProcessIdToSessionId(g_RunData.CliProcessId, &SessionID);
    SetTokenInformation(hUser, TokenSessionId, &SessionID, sizeof(DWORD));
  }
  void *Env = 0;
  if (CreateEnvironmentBlock(&Env, hUser, FALSE)) {
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    // Do not inherit Desktop from calling process, use Tokens Desktop
    TCHAR WinstaDesk[2 * MAX_PATH + 2];
    _stprintf(WinstaDesk, _T("%s\\%s"), g_RunData.WinSta, g_RunData.Desk);
    si.lpDesktop = WinstaDesk;
    if (MyCPAU(hUser, NULL, g_RunData.cmdLine, NULL, NULL, FALSE,
               CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED, Env,
               g_RunData.CurDir, &si, &pi)) {
    UsualWay:
      RetVal = RETVAL_OK;
      g_RunData.NewPID = pi.dwProcessId;
      // SetParentProcessID(pi.hProcess,GetParentProcessID(g_RunData.CliProcessId));
      CloseHandle(pi.hProcess);
      ResumeThread(pi.hThread);
      CloseHandle(pi.hThread);
      // ShellExec-Hook: We must return the PID and TID to fake CreateProcess:
      if ((g_RunData.RetPID) && (g_RunData.RetPtr)) {
        HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                                      FALSE, g_RunData.RetPID);
        if (hProcess) {
          SIZE_T n;
          RET_PROCESS_INFORMATION rpi;
          rpi.dwProcessId = pi.dwProcessId;
          rpi.dwThreadId = pi.dwThreadId;
          if (!WriteProcessMemory(hProcess, (LPVOID)g_RunData.RetPtr, &rpi,
                                  sizeof(rpi), &n))
            DBGTrace2("AutoSuRun(%s) WriteProcessMemory failed: %s",
                      g_RunData.cmdLine, GetLastErrorNameStatic());
          CloseHandle(hProcess);
        } else
          DBGTrace2("AutoSuRun(%s) OpenProcess failed: %s", g_RunData.cmdLine,
                    GetLastErrorNameStatic());
      }
    } else {
      if (GetLastError() == 740 /*ERROR_ELEVATION_REQUIRED*/) {
        // We need to start the process elevated and then drop it's rights
        HANDLE hAdmin = GetAdminToken(g_RunData.SessionID);
        BOOL bCloseAdmin = TRUE;
        if (!hAdmin) {
          bCloseAdmin = FALSE;
          hAdmin = GetTempAdminToken(g_RunData.UserName);
          SetTokenInformation(hAdmin, TokenSessionId, &g_RunData.SessionID,
                              sizeof(DWORD));
        }
        if (MyCPAU(hAdmin, NULL, g_RunData.cmdLine, NULL, NULL, FALSE,
                   CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED, Env,
                   g_RunData.CurDir, &si, &pi)) {
          // No need to set the thread token. Threads only use impersonation
          // tokens.
          if (SetProcessUserToken(pi.hProcess, hUser)) {
            if (bCloseAdmin)
              CloseHandle(hAdmin);
            goto UsualWay;
          }
          TerminateProcess(pi.hProcess, (DWORD)-1);
        } else
          DBGTrace1("CreateProcessAsUser failed: %s", GetLastErrorNameStatic());
        if (bCloseAdmin)
          CloseHandle(hAdmin);
      } else
        DBGTrace1("CreateProcessAsUser failed: %s", GetLastErrorNameStatic());
    }
    DestroyEnvironmentBlock(Env);
  }
  CloseHandle(hUser);
  return RetVal;
}

//////////////////////////////////////////////////////////////////////////////
//
//  SuRun
//
//////////////////////////////////////////////////////////////////////////////
void SuRun() {
  // #ifdef DoDBGTrace
  //   g_nTimes++;
  //   AddTime("Client to service comm.")
  // #endif //DoDBGTrace
  // This is called from a separate process created by the service
  if (!IsLocalSystem()) {
    DBGTrace("FATAL: SuRun() not running as SYSTEM; EXIT!");
    return;
  }
  zero(g_RunPwd);
  ActivateKeyboardLayout(GetKeyboardLayout(g_RunData.CliThreadId),
                         KLF_SETFORPROCESS);
  DWORD RetVal = RETVAL_ACCESSDENIED;
  // RunAs...
  if (g_RunData.bRunAs) {
    if (CreateSafeDesktop(g_RunData.WinSta, g_RunData.Desk, GetBlurDesk,
                          (!(g_RunData.Groups & IS_TERMINAL_USER)) &
                              GetFadeDesk)) {
      TCHAR User[UNLEN + DNLEN + 2] = {0};
      _tcscpy(User, g_RunData.UserName);
      TCHAR RunAsUser[UNLEN + DNLEN + 2] = {0};
      if (!GetRegStr(HKLM, USERKEY(g_RunData.UserName), L"LastRunAsUser",
                     RunAsUser, UNLEN + DNLEN + 2))
        _tcscpy(RunAsUser, g_RunData.UserName);
      DWORD r = RunAsLogon(g_RunData.SessionID, User, g_RunPwd, RunAsUser,
                           IDS_ASKRUNAS, BeautifyCmdLine(g_RunData.cmdLine));
      if (r == 0) {
        DeleteSafeDesktop((!(g_RunData.Groups & IS_TERMINAL_USER)) &
                          GetFadeDesk);
        ResumeClient(RETVAL_CANCELLED);
        return;
      }
      RetVal = RETVAL_OK;
      DeleteSafeDesktop(false);
      SetRegStr(HKLM, USERKEY(g_RunData.UserName), L"LastRunAsUser", User);
      _tcscpy(g_RunData.UserName, User);
      if (r & (1 << 5)) // /USER SYSTEM
      {
        ResumeClient(DirectStartUserProcess(GetProcessID(L"Services.exe")),
                     true);
        return;
      }
      // AsAdmin!
      if ((r & 0x08) == 0)
        g_RunData.bRunAs |= 2;
      else
        g_RunData.bRunAs &= ~2;
      if (r & 0x10) // Same user as Client Process
        g_RunData.bRunAs &= ~0x01;
    } else {
      DBGTrace("CreateSafeDesktop failed");
      ResumeClient(RETVAL_CANCELLED);
      if (!g_RunData.beQuiet)
        SafeMsgBox(0, CBigResStr(IDS_NODESK), CResStr(IDS_APPNAME),
                   MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
      return;
    }
  } else // if (!g_RunData.bRunAs)
  {
    // Do Fast User Switching?
    if (g_RunData.KillPID == 0xFFFFFFFF) {
      if (_tcsnicmp(g_RunData.cmdLine, _T("/SWITCHTO "), 10) == 0) {
        ResumeClient(DoFUS());
        return;
      }
      if (_tcsicmp(g_RunData.cmdLine, _T("/FUSGUI")) == 0) {
        ShowFUSGUI();
        return;
      }
      // Empty Trash?
      if ((_tcsicmp(g_RunData.cmdLine, _T("/EmptyRecycleBin")) == 0)) {
        if (((g_RunData.Groups & IS_IN_ADMINS) == 0) &&
            (((g_RunData.Groups & IS_IN_SURUNNERS) == 0) ||
             (GetRestrictApps(g_RunData.UserName)))) {
          ResumeClient(RETVAL_ACCESSDENIED);
          ExitProcess(0x0FFFFFFF & (~GetCurrentProcessId()));
          for (;;)
            ;
        }
        GetSystemWindowsDirectory(g_RunData.cmdLine, 4096);
        PathAppend(g_RunData.cmdLine, L"SuRun.exe");
        PathQuoteSpaces(g_RunData.cmdLine);
        _tcscat(g_RunData.cmdLine, L" /EmptyRecycleBin");
        ResumeClient(LSAStartAdminProcess(), true);
        return;
      }
    }
    // Setup?
    if (_tcsicmp(g_RunData.cmdLine, _T("/SETUP")) == 0) {
      // Create the new desktop
      ResumeClient(RETVAL_OK);
      // check if SuRun Setup is hidden for user name
      if (HideSuRun(g_RunData.UserName, g_RunData.Groups))
        return;
      bool bFadeDesk = (!(g_RunData.Groups & IS_TERMINAL_USER)) & GetFadeDesk;
      if (CreateSafeDesktop(g_RunData.WinSta, g_RunData.Desk, GetBlurDesk,
                            bFadeDesk)) {
        try {
          Setup();
        } catch(...) {
        }
        DeleteSafeDesktop(bFadeDesk);
      } else {
        if (!g_RunData.beQuiet)
          SafeMsgBox(0, CBigResStr(IDS_NODESK), CResStr(IDS_APPNAME),
                     MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
      }
      return;
    }
    if (g_CliIsAdmin && (GetNoConvAdmin || GetNoConvUser)) {
      // Just start the client process!
      KillProcess(g_RunData.KillPID);
      ResumeClient(DirectStartUserProcess(g_RunData.CliProcessId), true);
      return;
    }
    // Start execution
    RetVal = PrepareSuRun();
    if (RetVal != RETVAL_OK) {
      if (RetVal == RETVAL_NODESKTOP) {
        ResumeClient(g_RunData.bShlExHook ? RETVAL_SX_NOTINLIST
                                          : RETVAL_CANCELLED);
        if (!g_RunData.beQuiet)
          SafeMsgBox(0, CBigResStr(IDS_NODESK), CResStr(IDS_APPNAME),
                     MB_ICONSTOP | MB_SERVICE_NOTIFICATION);
        return;
      }
      if (g_RunData.bShlExHook &&
          (!IsInWhiteList(g_RunData.UserName, g_RunData.cmdLine,
                          FLAG_SHELLEXEC)))
        ResumeClient(RETVAL_SX_NOTINLIST); // let ShellExecute start the
                                           // process!
      else
        ResumeClient(RetVal);
      return;
    }
  } //(g_RunData.bRunAs)
  // copy the password to the client
  try {
    KillProcess(g_RunData.KillPID);
    RetVal = LSAStartAdminProcess();
  } catch(...) {
    DBGTrace("FATAL: Exception in StartAdminProcessTrampoline()");
  }
  ResumeClient(RetVal, true);
}

//////////////////////////////////////////////////////////////////////////////
//
//  AppInitDlls:
//
//////////////////////////////////////////////////////////////////////////////
#define AppInit32                                                              \
  _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Windows")
#define AppInit _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows")

static void RemoveAppInit(LPCTSTR Key, LPCTSTR Dll) {
  // remove from AppInit_Dlls
  TCHAR s[4096] = {0};
  GetRegStr(HKLM, Key, _T("AppInit_DLLs"), s, 4096);
  LPTSTR p = _tcsstr(s, Dll);
  if (p != 0) {
    LPTSTR p1 = p + _tcslen(Dll);
    if ((*p1 == ' ') || (*p1 == ','))
      p1++;
    if (p != s)
      p--;
    *p = 0;
    if (*(p1))
      _tcscat(p, p1);
    SetRegStr(HKLM, Key, _T("AppInit_DLLs"), s);
  }
}

// static void AddAppInit(LPCTSTR Key,LPCTSTR Dll)
// {
//   TCHAR s[4096]={0};
//   GetRegStr(HKLM,Key,_T("AppInit_DLLs"),s,4096);
//   if (_tcsstr(s,Dll)==0)
//   {
//     if (s[0])
//       _tcscat(s,_T(","));
//     _tcscat(s,Dll);
//     SetRegStr(HKLM,Key,_T("AppInit_DLLs"),s);
//   }
// }

//////////////////////////////////////////////////////////////////////////////
//
//  InstallRegistry
//
//////////////////////////////////////////////////////////////////////////////
BOOL g_bKeepRegistry = FALSE;
bool g_bDelSuRunners = TRUE;
bool g_bSR2Admins = FALSE;
HWND g_InstLog = 0;
#define InstLog(s)                                                             \
  {                                                                            \
    SendMessage(g_InstLog, LB_SETTOPINDEX,                                     \
                SendMessage(g_InstLog, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s),   \
                0);                                                            \
    MsgLoop();                                                                 \
    Sleep(250);                                                                \
  }

#define UNINSTL                                                                \
  L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SuRun"

#define SHLRUN L"\\Shell\\SuRun"
#define EXERUN L"exefile" SHLRUN
#define CMDRUN L"cmdfile" SHLRUN
#define CPLRUN L"cplfile" SHLRUN
#define MSCRUN L"mscfile" SHLRUN
#define BATRUN L"batfile" SHLRUN
#define MSIPTCH L"Msi.Patch" SHLRUN
#define MSIPKG L"Msi.Package" SHLRUN
#define REGRUN L"regfile" SHLRUN
#define CPLREG                                                                 \
  L"CLSID\\{21EC2020-3AEA-1069-A2DD-08002B30309D}" SHLRUN // Control Panel
#define TRSREG                                                                 \
  L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}" SHLRUN // Recycle Bin
#define TRSREG1                                                                \
  L"CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\Shell\\SuRunEmpty"

static void MsgLoop() {
  MSG msg;
  int count = 0;
  while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && (count++ < 100)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void InstallRegistry() {
  TCHAR SuRunExe[MAX_PATH];
  GetSystemWindowsDirectory(SuRunExe, MAX_PATH);
  InstLog(CResStr(IDS_ADDUNINST));
  PathAppend(SuRunExe, L"SuRun.exe");
  PathQuoteSpaces(SuRunExe);
  CResStr MenuStr(IDS_MENUSTR);
  CBigResStr DefCmd(L"%s \"%%1\" %%*", SuRunExe);
  // UnInstall
  SetRegStr(HKLM, UNINSTL, L"DisplayName", L"Super User Run (SuRun)");
  SetRegStr(HKLM, UNINSTL, L"UninstallString",
            CBigResStr(L"%s /UNINSTALL", SuRunExe, SuRunExe));
  SetRegStr(HKLM, UNINSTL, L"DisplayVersion", GetVersionString());
  SetRegStr(HKLM, UNINSTL, L"Publisher", L"Kay Bruns");
  SetRegStr(HKLM, UNINSTL, L"URLInfoAbout", L"http://kay-bruns.de/surun");
  SetRegStr(HKLM, UNINSTL, L"DisplayIcon", SuRunExe);
  if (_winmajor < 6) {
    // WinLogon Notification
    SetRegInt(HKLM, WINLOGONKEY, L"Asynchronous", 0);
    SetRegStr(HKLM, WINLOGONKEY, L"DllName", L"SuRunExt.dll");
    SetRegInt(HKLM, WINLOGONKEY, L"Impersonate", 0);
    SetRegStr(HKLM, WINLOGONKEY, L"Logoff", L"SuRunLogoffUser");
  }
  // AutoRun, System Menu Hook
  InstLog(CResStr(IDS_ADDAUTORUN));
  SetRegStr(HKLM, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            CResStr(IDS_SYSMENUEXT), CBigResStr(L"%s /SYSMENUHOOK", SuRunExe));
  if (!g_bKeepRegistry) {
    LPCTSTR iconstr = L"SuRunExt.dll,1";
    InstLog(CResStr(IDS_ADDASSOC));

    // exefile
    SetRegStr(HKCR, EXERUN, L"", MenuStr);
    SetRegStr(HKCR, EXERUN, L"Icon", iconstr);
    SetRegStr(HKCR, EXERUN L"\\command", L"", DefCmd);
    // cmdfile
    SetRegStr(HKCR, CMDRUN, L"", MenuStr);
    SetRegStr(HKCR, CMDRUN, L"Icon", iconstr);
    SetRegStr(HKCR, CMDRUN L"\\command", L"", DefCmd);
    // cplfile
    SetRegStr(HKCR, CPLRUN, L"", MenuStr);
    SetRegStr(HKCR, CPLRUN, L"Icon", iconstr);
    SetRegStr(HKCR, CPLRUN L"\\command", L"", DefCmd);
    // MSCFile
    SetRegStr(HKCR, MSCRUN, L"", MenuStr);
    SetRegStr(HKCR, MSCRUN, L"Icon", iconstr);
    SetRegStr(HKCR, MSCRUN L"\\command", L"", DefCmd);
    // batfile
    SetRegStr(HKCR, BATRUN, L"", MenuStr);
    SetRegStr(HKCR, BATRUN, L"Icon", iconstr);
    SetRegStr(HKCR, BATRUN L"\\command", L"", DefCmd);
    // regfile
    SetRegStr(HKCR, REGRUN, L"", MenuStr);
    SetRegStr(HKCR, REGRUN, L"Icon", iconstr);
    SetRegStr(HKCR, REGRUN L"\\command", L"", DefCmd);
    TCHAR MSIExe[MAX_PATH];
    GetSystemDirectory(MSIExe, MAX_PATH);
    PathAppend(MSIExe, L"msiexec.exe");
    PathQuoteSpaces(MSIExe);
    // MSI Install
    SetRegStr(HKCR, MSIPKG L" open", L"", CResStr(IDS_SURUNINST));
    SetRegStr(HKCR, MSIPKG L" open", L"Icon", iconstr);
    SetRegStr(HKCR, MSIPKG L" open\\command", L"",
              CBigResStr(L"%s %s /i \"%%1\" %%*", SuRunExe, MSIExe));
    // MSI Repair
    SetRegStr(HKCR, MSIPKG L" repair", L"", CResStr(IDS_SURUNREPAIR));
    SetRegStr(HKCR, MSIPKG L" repair", L"Icon", iconstr);
    SetRegStr(HKCR, MSIPKG L" repair\\command", L"",
              CBigResStr(L"%s %s /f \"%%1\" %%*", SuRunExe, MSIExe));
    // MSI Uninstall
    SetRegStr(HKCR, MSIPKG L" Uninstall", L"", CResStr(IDS_SURUNUNINST));
    SetRegStr(HKCR, MSIPKG L" Uninstall", L"Icon", iconstr);
    SetRegStr(HKCR, MSIPKG L" Uninstall\\command", L"",
              CBigResStr(L"%s %s /x \"%%1\" %%*", SuRunExe, MSIExe));
    // MSP Apply
    SetRegStr(HKCR, MSIPTCH L" open", L"", MenuStr);
    SetRegStr(HKCR, MSIPTCH L" open", L"Icon", iconstr);
    SetRegStr(HKCR, MSIPTCH L" open\\command", L"",
              CBigResStr(L"%s %s /p \"%%1\" %%*", SuRunExe, MSIExe));
    // Control Panel
    SetRegStr(HKCR, CPLREG, L"", MenuStr);
    SetRegStr(HKCR, CPLREG, L"Icon", iconstr);
    SetRegStr(HKCR, CPLREG L"\\command", L"",
              CBigResStr(L"%s control", SuRunExe));
    // Trash
    SetRegStr(HKCR, TRSREG, L"", MenuStr);
    SetRegStr(HKCR, TRSREG, L"Icon", iconstr);
    SetRegStr(
        HKCR, TRSREG L"\\command", L"",
        CBigResStr(L"%s Explorer /N, ::{645FF040-5081-101B-9F08-00AA002F954E}",
                   SuRunExe));
    SetRegStr(HKCR, TRSREG1, L"", CResStr(IDS_EMPTYRECYCLEBIN));
    SetRegStr(HKCR, TRSREG1, L"Icon", iconstr);
    SetRegStr(HKCR, TRSREG1 L"\\command", L"",
              CBigResStr(L"%s /EmptyRecycleBin", SuRunExe));
  }
  // Control Panel Applet
  InstLog(CResStr(IDS_ADDCPL));
  GetSystemWindowsDirectory(SuRunExe, MAX_PATH);
  PathAppend(SuRunExe, L"SuRunExt.dll");
  SetRegStr(
      HKLM,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls",
      L"SuRunCpl", SuRunExe);
  // Add SuRun CPL to "Performance and Maintenance"
  SetRegInt(
      HKLM,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended "
      L"Properties\\{305CA226-D286-468e-B848-2B2E8E697B74} 2",
      SuRunExe, 5);
  // add to AppInit_Dlls
#ifdef _TEST_STABILITY
  SetRegInt(HKLM, AppInit, _T("LoadAppInit_DLLs"), 1);
  AddAppInit(AppInit, _T("SuRunExt.dll"));
#ifdef _WIN64
  SetRegInt(HKLM, AppInit32, _T("LoadAppInit_DLLs"), 1);
  AddAppInit(AppInit32, _T("SuRunExt32.dll"));
#endif //_WIN64
#endif //_TEST_STABILITY
}

//////////////////////////////////////////////////////////////////////////////
//
//  RemoveRegistry
//
//////////////////////////////////////////////////////////////////////////////
void RemoveRegistry() {
  // AppInit_Dlls
  RemoveAppInit(AppInit, _T("SuRunExt.dll"));
#ifdef _WIN64
  RemoveAppInit(AppInit32, _T("SuRunExt32.dll"));
#endif //_WIN64
  if (!g_bKeepRegistry) {
    InstLog(CResStr(IDS_REMASSOC));
    // exefile
    DelRegKey(HKCR, EXERUN);
    // cmdfile
    DelRegKey(HKCR, CMDRUN);
    // cplfile
    DelRegKey(HKCR, CPLRUN);
    // MSCFile
    DelRegKey(HKCR, MSCRUN);
    // batfile
    DelRegKey(HKCR, BATRUN);
    // regfile
    DelRegKey(HKCR, REGRUN);
    // MSI Install
    DelRegKey(HKCR, MSIPKG L" open");
    // MSI Repair
    DelRegKey(HKCR, MSIPKG L" repair");
    // MSI Uninstall
    DelRegKey(HKCR, MSIPKG L" Uninstall");
    // MSP Apply
    DelRegKey(HKCR, MSIPTCH L" open");
    // Control Panel
    DelRegKey(HKCR, CPLREG);
    // Trash
    DelRegKey(HKCR, TRSREG);
    DelRegKey(HKCR, TRSREG1);
  }
  // Control Panel Applet
  InstLog(CResStr(IDS_REMCPL));
  RegDelVal(
      HKLM,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls",
      L"SuRunCpl");
  TCHAR CPL[4096];
  GetSystemWindowsDirectory(CPL, 4096);
  PathAppend(CPL, L"SuRunExt.dll");
  RegDelVal(
      HKLM,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Extended "
      L"Properties\\{305CA226-D286-468e-B848-2B2E8E697B74} 2",
      CPL);
  // AutoRun, System Menu Hook
  InstLog(CResStr(IDS_REMAUTORUN));
  RegDelVal(HKLM, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            CResStr(IDS_SYSMENUEXT));
  // WinLogon Notification
  if (_winmajor < 6)
    DelRegKey(HKLM, WINLOGONKEY);
  // UnInstall
  InstLog(CResStr(IDS_REMUNINST));
  DelRegKey(HKLM, UNINSTL);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Service control helpers
//
//////////////////////////////////////////////////////////////////////////////

#define WaitFor(a)                                                             \
  {                                                                            \
    for (int i = 0; i < 30; i++) {                                             \
      Sleep(750);                                                              \
      if (a)                                                                   \
        return TRUE;                                                           \
    }                                                                          \
    return a;                                                                  \
  }

BOOL RunThisAsAdmin(LPCTSTR cmd, DWORD WaitStat, int nResId) {
  TCHAR ModName[MAX_PATH];
  TCHAR cmdLine[4096];
  GetProcessFileName(ModName, MAX_PATH);
  NetworkPathToUNCPath(ModName);
  PathQuoteSpaces(ModName);
  TCHAR User[UNLEN + GNLEN + 2] = {0};
  GetProcessUserName(GetCurrentProcessId(), User);
  DWORD dwSess = 0;
  ProcessIdToSessionId(GetCurrentProcessId(), &dwSess);
  if (IsInSuRunners(User, dwSess) &&
      (CheckServiceStatus() == SERVICE_RUNNING)) {
    DBGTrace2("RunThisAsAdmin %s is SuRunner: starting %s with SuRun", User,
              cmd);
    TCHAR SvcFile[4096];
    GetSystemWindowsDirectory(SvcFile, 4096);
    PathAppend(SvcFile, _T("SuRun.exe"));
    PathQuoteSpaces(SvcFile);
    _stprintf(cmdLine, _T("%s /QUIET %s %s"), SvcFile, ModName, cmd);
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);
    if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si,
                      &pi)) {
      CloseHandle(pi.hThread);
      DWORD ExitCode = (DWORD)-1;
      if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_OBJECT_0)
        GetExitCodeProcess(pi.hProcess, &ExitCode);
      CloseHandle(pi.hProcess);
      if (ExitCode != RETVAL_OK)
        return FALSE;
      WaitFor(CheckServiceStatus() == WaitStat);
    } else
      DBGTrace2("RunThisAsAdmin CreateProcess(%s) failed: %s", cmd,
                GetLastErrorNameStatic());
  }
  _stprintf(cmdLine, _T("%s %s"), ModName, cmd);
  if (!RunAsAdmin(cmdLine, nResId))
    return FALSE;
  WaitFor(CheckServiceStatus() == WaitStat);
}

void DelFile(LPCTSTR File) {
  InstLog(CBigResStr(IDS_DELFILE, File));
  if (PathFileExists(File) && (!DeleteFile(File))) {
    CBigResStr tmp(_T("%s.tmp"), File);
    DeleteFile(tmp);
    while (_trename(File, tmp)) {
      _tcscat(tmp, L".tmp");
      DeleteFile(tmp);
    }
    InstLog(CBigResStr(IDS_DELFILEBOOT, (LPCTSTR)tmp));
    MoveFileEx(tmp, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
  }
}

void CopyToWinDir(LPCTSTR File) {
  TCHAR DstFile[4096];
  TCHAR SrcFile[4096];
  GetProcessFileName(SrcFile, MAX_PATH);
  NetworkPathToUNCPath(SrcFile);
  PathRemoveFileSpec(SrcFile);
  PathAppend(SrcFile, File);
  GetSystemWindowsDirectory(DstFile, 4096);
  PathAppend(DstFile, File);
  DelFile(DstFile);
  InstLog(CBigResStr(IDS_COPYFILE, SrcFile, DstFile));
  CopyFile(SrcFile, DstFile, FALSE);
}

// the real /UNINSTALL implementation:
BOOL DeleteService(BOOL bJustStop = FALSE) {
  BOOL bRet = FALSE;
  InstLog(CResStr(IDS_DELSERVICE));
  SC_HANDLE hdlSCM = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
  if (hdlSCM) {
    SC_HANDLE hdlServ =
        OpenService(hdlSCM, GetSvcName(), SERVICE_STOP | DELETE);
    if (hdlServ) {
      SERVICE_STATUS ss;
      ControlService(hdlServ, SERVICE_CONTROL_STOP, &ss);
      DeleteService(hdlServ);
      CloseServiceHandle(hdlServ);
      bRet = TRUE;
    }
  }
  for (int n = 0; CheckServiceStatus() && (n < 100); n++)
    Sleep(100);
  // Delete Files and directories
  TCHAR File[4096];
  GetSystemWindowsDirectory(File, 4096);
  PathAppend(File, _T("SuRun.exe"));
  DelFile(File);
  GetSystemWindowsDirectory(File, 4096);
  PathAppend(File, _T("SuRunExt.dll"));
  DelFile(File);
#ifdef _WIN64
  GetSystemWindowsDirectory(File, 4096);
  PathAppend(File, _T("SuRun32.bin"));
  DelFile(File);
  GetSystemWindowsDirectory(File, 4096);
  PathAppend(File, _T("SuRunExt32.dll"));
  DelFile(File);
#endif //_WIN64
  // Start Menu
  TCHAR file[4096];
  GetRegStr(HKLM,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User "
            L"Shell Folders",
            L"Common Programs", file, 4096);
  ExpandEnvironmentStrings(file, File, 4096);
  PathAppend(File, CResStr(IDS_STARTMENUDIR));
  DeleteDirectory(File);
  // Registry
  RemoveRegistry();
  if (bJustStop)
    return TRUE;
  // restore RunAs
  ReplaceSuRunWithRunAs();
  // Restore Windows Options
  if (SRGetWinUpdBoot && GetWinUpdBoot)
    SetWinUpdBoot(false);
  if (SRGetWinUpd4All && GetWinUpd4All)
    SetWinUpd4All(false);
  if (SRGetOwnerAdminGrp && GetOwnerAdminGrp)
    SetOwnerAdminGrp(false);
  SetSetEnergy(false);
  SetSetTime(SURUNNERSGROUP, false);
  if (!g_bKeepRegistry) {
    InstLog(CResStr(IDS_DELREG));
    // remove COM Object Settings
    DelRegKey(HKCR, L"CLSID\\" sGUID);
    // Remove SuRunners from Registry
    // HKLM\Security\SuRun
    SetRegistryTreeAccess(_T("MACHINE\\") SVCKEY, DOMAIN_ALIAS_RID_ADMINS,
                          true);
    DelRegKey(HKLM, SVCKEY);
    // HKLM\Software\SuRun
    DelRegKey(HKLM, SURUNKEY);
  }
  // Delete "SuRunners"?
  if (g_bDelSuRunners) {
    // Make "SuRunners"->"Administrators"?
    if (g_bSR2Admins) {
      USERLIST SuRunners;
      SuRunners.SetSurunnersUsers(0, (DWORD)-1, FALSE);
      for (int i = 0; i < SuRunners.GetCount(); i++) {
        InstLog(CResStr(IDS_SR2ADMIN, SuRunners.GetUserName(i)));
        AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, SuRunners.GetUserName(i), 1);
      }
    }
    InstLog(CResStr(IDS_DELSURUNNERS));
    DeleteSuRunnersGroup();
  }
  return bRet;
}

BOOL InstallService() {
  if (!IsAdmin())
    return RunThisAsAdmin(_T("/INSTALL"), SERVICE_RUNNING, IDS_INSTALLADMIN);
  if (CheckServiceStatus()) {
    if (!DeleteService(true)) {
      DBGTrace1("DeleteService failed: %s", GetLastErrorNameStatic());
      return FALSE;
    }
    // Wait until "SuRun /SYSMENUHOOK" has exited:
    CTimeOut t(11000);
    for (;;) {
      if (t.TimedOut())
        break;
      HANDLE m = CreateMutex(NULL, true, _T("SuRun_SysMenuHookIsRunning"));
      if (GetLastError() != ERROR_ALREADY_EXISTS) {
        CloseHandle(m);
        break;
      }
      CloseHandle(m);
      Sleep(200);
    }
  }
  SC_HANDLE hdlSCM = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
  if (hdlSCM == 0)
    return FALSE;
  CopyToWinDir(_T("SuRun.exe"));
  CopyToWinDir(_T("SuRunExt.dll"));
#ifdef _WIN64
  CopyToWinDir(_T("SuRun32.bin"));
  CopyToWinDir(_T("SuRunExt32.dll"));
#endif //_WIN64
  InstLog(CResStr(IDS_INSTSVC));
  TCHAR SvcFile[4096];
  GetSystemWindowsDirectory(SvcFile, 4096);
  PathAppend(SvcFile, _T("SuRun.exe"));
  PathQuoteSpaces(SvcFile);
  _tcscat(SvcFile, _T(" /ServiceRun"));
  SC_HANDLE hdlServ = CreateService(
      hdlSCM, GetSvcName(), SvcDisplayText, STANDARD_RIGHTS_REQUIRED,
      SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
      SvcFile, L"SpoolerGroup", 0, 0, 0, 0);
  if (!hdlServ) {
    DBGTrace1("CreateService failed: %s", GetLastErrorNameStatic());
    CloseServiceHandle(hdlSCM);
    return FALSE;
  }
  CloseServiceHandle(hdlServ);
  InstLog(CResStr(IDS_STARTSVC));
  hdlServ = OpenService(hdlSCM, GetSvcName(), SERVICE_START);
  if (!hdlServ)
    DBGTrace1("OpenService failed: %s", GetLastErrorNameStatic());
  BOOL bRet = StartService(hdlServ, 0, 0);
  if (!bRet) {
    DBGTrace1("StartService failed: %s", GetLastErrorNameStatic());
    CloseServiceHandle(hdlServ);
    hdlServ = OpenService(hdlSCM, GetSvcName(), SERVICE_STOP | DELETE);
    if (hdlServ)
      DeleteService(hdlServ);
  }
  CloseServiceHandle(hdlServ);
  CloseServiceHandle(hdlSCM);
  if (bRet) {
    // Registry
    InstallRegistry();
    //"SuRunners" Group
    CreateSuRunnersGroup();
    WaitFor(CheckServiceStatus() == SERVICE_RUNNING);
  }
  return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
// Install/Update Dialogs Proc
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK InstallDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  switch (msg) {
  case WM_INITDIALOG: {
    BringToPrimaryMonitor(hwnd);
    SendMessage(hwnd, WM_SETICON, ICON_BIG,
                (LPARAM)LoadImage(GetModuleHandle(0),
                                  MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32,
                                  32, 0));
    SendMessage(hwnd, WM_SETICON, ICON_SMALL,
                (LPARAM)LoadImage(GetModuleHandle(0),
                                  MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16,
                                  16, 0));
    {
      TCHAR WndText[MAX_PATH] = {0}, newText[MAX_PATH] = {0};
      GetWindowText(hwnd, WndText, MAX_PATH);
      _stprintf(newText, WndText, GetVersionString());
      SetWindowText(hwnd, newText);
    }
    SendDlgItemMessage(hwnd, IDC_QUESTION, WM_SETFONT,
                       (WPARAM)CreateFont(-24, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0,
                                          0, 0, 0, _T("MS Shell Dlg")),
                       1);
    CheckDlgButton(hwnd, IDC_RUNSETUP, 1);
    g_bKeepRegistry = GetSROption(L"UpdKeepSettings", 0);
    CheckDlgButton(hwnd, IDC_KEEPREGISTRY, g_bKeepRegistry);
    CheckDlgButton(hwnd, IDC_OWNERGROUP, 1);
    if (GetOwnerAdminGrp) {
      ShowWindow(GetDlgItem(hwnd, IDC_OWNERGROUP), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDC_OWNGRPST), SW_HIDE);
    }
    CheckDlgButton(hwnd, IDC_DELSURUNNERS, g_bDelSuRunners);
    CheckDlgButton(hwnd, IDC_SR2ADMIN, g_bSR2Admins);
    g_InstLog = GetDlgItem(hwnd, IDC_INSTLOG);
    return FALSE;
  } // WM_INITDIALOG
  case WM_DESTROY: {
    HGDIOBJ fs = GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ f =
        (HGDIOBJ)SendDlgItemMessage(hwnd, IDC_QUESTION, WM_GETFONT, 0, 0);
    SendDlgItemMessage(hwnd, IDC_QUESTION, WM_SETFONT, (WPARAM)fs, 0);
    if (f)
      DeleteObject(f);
    return TRUE;
  }
  case WM_CTLCOLORSTATIC: {
    int CtlId = GetDlgCtrlID((HWND)lParam);
    if ((CtlId == IDC_QUESTION) || (CtlId == IDC_SECICON)) {
      SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
      SetBkMode((HDC)wParam, TRANSPARENT);
      return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
    }
  } break;
  case WM_NCDESTROY: {
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
    return TRUE;
  } // WM_NCDESTROY
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(IDOK, BN_CLICKED): // Install SuRun:
      if (GetDlgItem(hwnd, IDCONTINUE))
        goto DoContinue;
      if (GetDlgItem(hwnd, IDCLOSE))
        goto DoClose;
      if (GetDlgItem(hwnd, IDOK)) {

        // Make IDOK->Close
        SetWindowLongPtr(GetDlgItem(hwnd, IDOK), GWL_ID, IDCLOSE);
        // Disable Buttons
        SetWindowLong(g_InstLog, GWL_STYLE,
                      GetWindowLong(g_InstLog, GWL_STYLE) | WS_TABSTOP);
        SetFocus(g_InstLog);
        EnableWindow(GetDlgItem(hwnd, IDCLOSE), 0);
        EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
        // remove all pending WM_COMMAND messages
        MSG msg;
        while (PeekMessage(&msg, hwnd, WM_COMMAND, WM_COMMAND, PM_REMOVE))
          ;
        // Settings
        g_bKeepRegistry = IsDlgButtonChecked(hwnd, IDC_KEEPREGISTRY) != 0;
        SetSROption(L"UpdKeepSettings", (DWORD)g_bKeepRegistry, 0);
        if (IsDlgButtonChecked(hwnd, IDC_OWNERGROUP))
          SetOwnerAdminGrp(1);
        // Hide Checkboxes, show Listbox
        ShowWindow(GetDlgItem(hwnd, IDC_RUNSETUP), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KEEPREGST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_KEEPREGISTRY), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_OWNERGROUP), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_OWNGRPST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_OPTNST), SW_HIDE);
        ShowWindow(g_InstLog, SW_SHOW);
        // Show some Progress
        SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDC_INSTALL));
        MsgLoop();
        if (!InstallService()) {
          // Install failed:
          InstLog(CResStr(IDS_INSTFAILED));
          SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDS_INSTFAILED));
          // hide IDCANCEL
          SetDlgItemText(hwnd, IDCLOSE, CResStr(IDS_CLOSE));
          ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
          EnableWindow(GetDlgItem(hwnd, IDCLOSE), 1);
          return TRUE;
        }
        // Run Setup?
        if (IsDlgButtonChecked(hwnd, IDC_RUNSETUP) != 0) {
          TCHAR SuRunExe[4096];
          GetSystemWindowsDirectory(SuRunExe, 4096);
          PathAppend(SuRunExe, L"SuRun.exe /SETUP");
          PROCESS_INFORMATION pi = {0};
          STARTUPINFO si = {0};
          si.cb = sizeof(si);
          if (CreateProcess(NULL, (LPTSTR)SuRunExe, NULL, NULL, FALSE,
                            NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hThread);
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
          }
        }
        // Show success
        SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDS_INSTSUCCESS));
        // Show "need logoff"
        InstLog(_T(" "));
        if (_winmajor < 6)
          InstLog(CResStr(IDS_INSTSUCCESS3)) else InstLog(
              CResStr(IDS_INSTSUCCESS2));
        // Enable OK, CANCEL
        EnableWindow(GetDlgItem(hwnd, IDCLOSE), 1);
        EnableWindow(GetDlgItem(hwnd, IDCANCEL), 1);
        // Cancel->Close; OK->Logoff
        SetWindowLongPtr(GetDlgItem(hwnd, IDCANCEL), GWL_ID, IDCLOSE);
        SetWindowLongPtr(GetDlgItem(hwnd, IDCLOSE), GWL_ID, IDCONTINUE);
        OSVERSIONINFO oie;
        oie.dwOSVersionInfoSize = sizeof(oie);
        if (_winmajor < 6)
          // 2k/XP Reboot required for WinLogon Notification
          SetDlgItemText(hwnd, IDCONTINUE, CResStr(IDS_REBOOT));
        else
          // Vista++ display LogOff
          SetDlgItemText(hwnd, IDCONTINUE, CResStr(IDC_LOGOFF));
        SetDlgItemText(hwnd, IDCLOSE, CResStr(IDS_CLOSE));
        SetFocus(GetDlgItem(hwnd, IDCONTINUE));
        SetWindowLong(g_InstLog, GWL_STYLE,
                      GetWindowLong(g_InstLog, GWL_STYLE) & (~WS_TABSTOP));
        return TRUE;
      }
    case MAKELPARAM(IDCANCEL, BN_CLICKED): // Close Dlg
      EndDialog(hwnd, IDCANCEL);
      return TRUE;
    case MAKELPARAM(IDCLOSE, BN_CLICKED): // Close Dlg
    DoClose:
      EndDialog(hwnd, IDCLOSE);
      return TRUE;
    case MAKELPARAM(IDCONTINUE, BN_CLICKED): // LogOff
    DoContinue:
      // ExitWindowsEx will not work here because we run as Admin
      if (_winmajor < 6) {
        // 2k/XP Reboot required for WinLogon Notification
        // ExitWindowsEx(EWX_REBOOT|EWX_FORCE,SHTDN_REASON_MINOR_RECONFIG);
        EnablePrivilege(SE_SHUTDOWN_NAME);
        InitiateSystemShutdown(0, 0, 0, true, true);
      } else
        // Vista++ no WinLogon Notification, just LogOff
        WTSLogoffSession(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, 0);
      EndDialog(hwnd, IDCONTINUE);
      return TRUE;
    case MAKELPARAM(IDIGNORE, BN_CLICKED): // Remove SuRun:
    {
      // Settings
      g_bKeepRegistry = (IsDlgButtonChecked(hwnd, IDC_KEEPREGISTRY) != 0);
      SetSROption(L"UpdKeepSettings", (DWORD)g_bKeepRegistry, 0);
      if (GetDlgItem(hwnd, IDC_DELSURUNNERS)) // Uninstall:
      {
        g_bDelSuRunners = (!g_bKeepRegistry) &&
                          (IsDlgButtonChecked(hwnd, IDC_DELSURUNNERS) != 0);
        g_bSR2Admins =
            g_bDelSuRunners && (IsDlgButtonChecked(hwnd, IDC_SR2ADMIN) != 0);
      } else // Update:
      {
        g_bDelSuRunners = FALSE;
        g_bSR2Admins = FALSE;
      }
      // Hide Checkboxes, show Listbox
      ShowWindow(GetDlgItem(hwnd, IDC_KEEPREGISTRY), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDC_DELSURUNNERS), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDC_SR2ADMIN), SW_HIDE);
      ShowWindow(GetDlgItem(hwnd, IDC_OPTNST), SW_HIDE);
      ShowWindow(g_InstLog, SW_SHOW);
      // Disable Buttons
      EnableWindow(GetDlgItem(hwnd, IDIGNORE), 0);
      EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
      // Show some Progress
      SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDC_UNINSTALL));
      // Make IDIGNORE->Close, hide IDCANCEL
      SetDlgItemText(hwnd, IDIGNORE, CResStr(IDS_CLOSE));
      SetWindowLongPtr(GetDlgItem(hwnd, IDIGNORE), GWL_ID, IDCLOSE);
      ShowWindow(GetDlgItem(hwnd, IDCANCEL), SW_HIDE);
      MsgLoop();
      if (!DeleteService(FALSE)) {
        // Install failed:
        InstLog(CResStr(IDS_UNINSTFAILED));
        SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDS_UNINSTFAILED));
        // Enable CLOSE
        EnableWindow(GetDlgItem(hwnd, IDCLOSE), 1);
        return TRUE;
      }
      // Show success
      SetDlgItemText(hwnd, IDC_QUESTION, CResStr(IDS_UNINSTSUCCESS));
      InstLog(_T(" "));
      InstLog(CResStr(IDS_UNINSTSUCCESS));
      // Enable CLOSE
      EnableWindow(GetDlgItem(hwnd, IDCLOSE), 1);
      return TRUE;
    }
    case MAKELPARAM(IDC_KEEPREGISTRY, BN_CLICKED):
      g_bKeepRegistry = IsDlgButtonChecked(hwnd, IDC_KEEPREGISTRY) != 0;
      SetSROption(L"UpdKeepSettings", (DWORD)g_bKeepRegistry, 0);
      EnableWindow(GetDlgItem(hwnd, IDC_DELSURUNNERS), !g_bKeepRegistry);
      // fall through:
    case MAKELPARAM(IDC_DELSURUNNERS, BN_CLICKED):
      EnableWindow(GetDlgItem(hwnd, IDC_SR2ADMIN),
                   IsDlgButtonChecked(hwnd, IDC_DELSURUNNERS) &&
                       (!g_bKeepRegistry));
      return TRUE;
    } // switch (wParam)
    break;
  } // WM_COMMAND
  }
  return FALSE;
}

BOOL UserInstall() {
  if (!IsAdmin()) {
    DBGTrace("UserInstall: No Admin! starting SuRun /USERINST as Admin");
    return RunThisAsAdmin(_T("/USERINST"), SERVICE_RUNNING, IDS_INSTALLADMIN);
  }
  return DialogBox(GetModuleHandle(0),
                   MAKEINTRESOURCE((CheckServiceStatus() != 0) ? IDD_UPDATE
                                                               : IDD_INSTALL),
                   0, InstallDlgProc) != IDCANCEL;
}

BOOL UserUninstall() {
  if (!IsAdmin())
    return RunThisAsAdmin(_T("/UNINSTALL"), 0, IDS_UNINSTALLADMIN);
  return DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_UNINSTALL), 0,
                   InstallDlgProc) != IDCANCEL;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUS Hotkey Thread
//
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI HKThreadProc(void *p) {
  UNREFERENCED_PARAMETER(p);
  MSG msg = {0};
  // Create Message queue
  ATOM HotKeyID = GlobalAddAtom(L"SuRunFUSHotKey");
  PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
  if (RegisterHotKey(0, HotKeyID, MOD_WIN, VK_ESCAPE)) {
    while (GetMessage(&msg, 0, WM_HOTKEY, WM_HOTKEY)) {
      g_RunData.KillPID = 0xFFFFFFFF;
      _tcscpy(g_RunData.cmdLine, _T("/FUSGUI"));
      HANDLE hPipe =
          CreateFile(ServicePipeName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
      if (hPipe != INVALID_HANDLE_VALUE) {
        DWORD n = 0;
        WriteFile(hPipe, &g_RunData, sizeof(RUNDATA), &n, 0);
        CloseHandle(hPipe);
      }
    }
    UnregisterHotKey(0, HotKeyID);
    GlobalDeleteAtom(HotKeyID);
  } else
    DBGTrace1("RegisterHotKey() failed: %s", GetLastErrorNameStatic());
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// SysmenuHook stuff
//
//////////////////////////////////////////////////////////////////////////////
static void HandleHooks() {
#ifdef _WIN64
  CreateMutex(NULL, true, _T("SuRun64_SysMenuHookIsRunning"));
#else //_WIN64
  CreateMutex(NULL, true, _T("SuRun_SysMenuHookIsRunning"));
#endif //_WIN64
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    return;
  g_RunData.CliProcessId = GetCurrentProcessId();
  g_RunData.CliThreadId = GetCurrentThreadId();
  GetWinStaName(g_RunData.WinSta, countof(g_RunData.WinSta));
  GetDesktopName(g_RunData.Desk, countof(g_RunData.Desk));
  GetProcessUserName(g_RunData.CliProcessId, g_RunData.UserName);
  ProcessIdToSessionId(g_RunData.CliProcessId, &g_RunData.SessionID);
  g_RunData.Groups = UserIsInSuRunnersOrAdmins();
  // ToDo: EnumProcesses,EnumProcessModules,GetModuleFileNameEx to check if the
  // hooks are still loaded

  // In the first three Minutes after Sytstem start:
  // Wait for the service to start
  DWORD ss = CheckServiceStatus();
  if ((ss == SERVICE_STOPPED) || (ss == SERVICE_START_PENDING))
    while ((GetTickCount() < 3 * 60 * 1000) &&
           (CheckServiceStatus() != SERVICE_RUNNING))
      Sleep(1000);
#ifdef _WIN64
  {
    TCHAR SuRun32Exe[4096];
    GetSystemWindowsDirectory(SuRun32Exe, 4096);
    PathAppend(SuRun32Exe, L"SuRun32.bin /SYSMENUHOOK");
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi;
    si.cb = sizeof(si);
    if (CreateProcess(NULL, SuRun32Exe, NULL, NULL, FALSE, 0, NULL, NULL, &si,
                      &pi)) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
  }
#endif //_WIN64
  // DWORD HkTID=0;
  // HANDLE hHkThread=CreateThread(0,0,HKThreadProc,0,0,&HkTID);
  CTimeOut t;
  HKEY RegKey = 0;
  RegOpenKeyEx(HKCR, L"CLSID\\" sGUID, 0, KSAM(KEY_READ), &RegKey);
  HANDLE hEvent = CreateEvent(0, 1, 1, 0);
  BOOL bShowTray = TRUE;
  BOOL bBaloon = FALSE;
  BOOL bHooked = FALSE;
  BOOL bAdmin = g_CliIsInAdmins;
#ifndef _SR32
  InitTrayShowAdmin();
#endif //_SR32
  for (;;) {
    if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
      ResetEvent(hEvent);
      bShowTray =
          ShowTray(g_RunData.UserName, g_CliIsInAdmins, g_CliIsInSuRunners);
      bBaloon =
          ShowBalloon(g_RunData.UserName, g_CliIsInAdmins, g_CliIsInSuRunners);
      BOOL bHook =
          (!bAdmin) && (GetRestartAsAdmin || GetStartAsAdmin || GetUseIATHook);
      if (bHook && (!bHooked)) {
        InstallSysMenuHook();
        bHooked = TRUE;
      } else if (bHooked && (!bHook)) {
        UninstallSysMenuHook();
        bHooked = FALSE;
      }
      RegNotifyChangeKeyValue(
          RegKey, 1, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
          hEvent, TRUE);
    }
    if (t.TimedOut()) {
      DWORD ss = CheckServiceStatus();
      if (ss != SERVICE_RUNNING) {
        DBGTrace1("SysmenuHook service not running(%X)==>Exit", ss);
        break;
      }
      t.Set(10000);
    }
#ifndef _SR32
    if (!ProcessTrayShowAdmin(bShowTray, bBaloon)) {
      DBGTrace("SysmenuHook ProcessTrayShowAdmin returned FALSE==>Exit");
      break;
    }
  }
  CloseTrayShowAdmin();
#else //_SR32
    Sleep(1000);
  }
#endif //_SR32
  UninstallSysMenuHook();
  //  if(WaitForSingleObject(hHkThread,0)==WAIT_TIMEOUT)
  //    PostThreadMessage(HkTID,WM_QUIT,0,0);
  //  if(WaitForSingleObject(hHkThread,1000)==WAIT_TIMEOUT)
  //    TerminateThread(hHkThread,-1);
  //  CloseHandle(hHkThread);
}

//////////////////////////////////////////////////////////////////////////////
//
// HandleServiceStuff: called on App Entry before WinMain()
//
//////////////////////////////////////////////////////////////////////////////
bool HandleServiceStuff() {
  INITCOMMONCONTROLSEX icce = {sizeof(icce),
                               ICC_USEREX_CLASSES | ICC_WIN95_CLASSES};
  InitCommonControlsEx(&icce);
  CCmdLine cmd(0);
  if (cmd.argc() == 3) {
    // Install
    if (_tcsicmp(cmd.argv(1), _T("/INSTALL")) == 0) {
      InstallService();
      TCHAR CMD[4096] = {0};
      GetSystemWindowsDirectory(CMD, 4096);
      PathAppend(CMD, L"SuRun.exe");
      PathQuoteSpaces(CMD);
      _tcscat(CMD, L" /RESTORE ");
      _tcscat(CMD, cmd.argv(2));
      STARTUPINFO si = {0};
      PROCESS_INFORMATION pi;
      si.cb = sizeof(si);
      if (CreateProcess(NULL, CMD, NULL, NULL, FALSE, 0, NULL, NULL, &si,
                        &pi)) {
        CloseHandle(pi.hThread);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
      }
      ExitProcess(0);
      for (;;)
        ;
    }
  }
  if (cmd.argc() == 2) {
    // Service GUI run in Users WindowStation
    if (_tcscmp(cmd.argv(1), _T("/AskUSER")) == 0) {
      SuRun();
      DeleteSafeDesktop(false);
      ExitProcess(0x0FFFFFFF & (~GetCurrentProcessId()));
      for (;;)
        ;
    }
    // Service
    if (_tcsicmp(cmd.argv(1), _T("/SERVICERUN")) == 0) {
      if (!IsLocalSystem())
        return false;
      // Shell Extension
      InstallShellExt();
      SERVICE_TABLE_ENTRY DispatchTable[] = {{GetSvcName(), ServiceMain},
                                             {0, 0}};
      // StartServiceCtrlDispatcher is a blocking call
      StartServiceCtrlDispatcher(DispatchTable);
      // Shell Extension
      RemoveShellExt();
      ExitProcess(0);
      for (;;)
        ;
    }
    // System Menu Hook: This is AutoRun for every user
    if (_tcsicmp(cmd.argv(1), _T("/SYSMENUHOOK")) == 0) {
      HandleHooks();
      ExitProcess(0);
      for (;;)
        ;
    }
    // Install
    if (_tcsicmp(cmd.argv(1), _T("/INSTALL")) == 0) {
      InstallService();
      ExitProcess(0);
      for (;;)
        ;
    }
    // UnInstall
    if (_tcsicmp(cmd.argv(1), _T("/UNINSTALL")) == 0) {
      UserUninstall();
      ExitProcess(0);
      for (;;)
        ;
    }
    // UserInst:
    if (_tcsicmp(cmd.argv(1), _T("/USERINST")) == 0) {
      UserInstall();
      ExitProcess(0);
      for (;;)
        ;
    }
  }
  if (cmd.argc() == 5) {
    // WatchDog
    if (_tcsicmp(cmd.argv(1), _T("/WATCHDOG")) == 0) {
      DoWatchDog(cmd.argv(2), cmd.argv(3), _tcstoul(cmd.argv(4), 0, 10));
      ExitProcess(0);
      for (;;)
        ;
    }
  }
  // In the first three Minutes after Sytstem start:
  // Wait for the service to start
  DWORD ss = CheckServiceStatus();
  if ((ss == SERVICE_STOPPED) || (ss == SERVICE_START_PENDING))
    while ((GetTickCount() < 3 * 60 * 1000) &&
           (CheckServiceStatus() != SERVICE_RUNNING))
      Sleep(1000);
  // The Service must be running!
  ss = CheckServiceStatus();
  if (ss != SERVICE_RUNNING) {
    ExitProcess((DWORD)-2); // Let ShellExec Hook return
    for (;;)
      ;
  }
  return false;
}
