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

#pragma once

#include <Psapi.h>
// #include "DBGTrace.H"

#define countof(b) sizeof(b) / sizeof(b[0])
#define zero(x) memset(&x, 0, sizeof(x))

#define StrLenW(s) (s ? wcslen(s) : 0)
#define StrLenA(s) (s ? strlen(s) : 0)

#define CHK_BOOL_FN(p)                                                         \
  if (!p)                                                                      \
    DBGTrace2("%s failed: %s", _T(#p), GetLastErrorNameStatic());

#if _MSC_VER >= 1500
extern unsigned int _osplatform;
extern unsigned int _osver;
extern unsigned int _winver;
extern unsigned int _winmajor;
extern unsigned int _winminor;
#endif //_MSC_VER >= 1500

#define IsWin2k (_winver == 0x500)
#define IsVista (_winver == 0x600)
#define IsVistapp (_winver >= 0x600)
#define IsWin7 (_winver == 0x601)
#define IsWin7pp (_winver >= 0x601)
#define IsWin8 (_winver == 0x602)
#define IsWin8pp (_winver >= 0x602)
#define IsWin81 (_winver == 0x603)
#define IsWin81pp (_winver >= 0x603)
#define IsWin10 (_winver == 0xA00)
#define IsWin10pp (_winver >= 0xA00)
#define IsWin1709pp ((_winver >= 0xA00) && (_osver >= 16299))

LANGID SetLocale(LANGID locale);

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION (0x1000)
#endif PROCESS_QUERY_LIMITED_INFORMATION

//  Registry Helper
#define HKCR HKEY_CLASSES_ROOT
#define HKCU HKEY_CURRENT_USER
#define HKLM HKEY_LOCAL_MACHINE
#define APP_PATHS L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths"

#ifdef _SR32
#define KSAM(a) a | KEY_WOW64_64KEY
#else
#define KSAM(a) a
#endif _SR32

#ifndef SS_REALSIZECONTROL
#define SS_REALSIZECONTROL 0x00000040L
#endif SS_REALSIZECONTROL

bool RegSetDACL(HKEY HK, LPCTSTR SubKey, PACL pACL);
PACL RegGrantAdminAccess(HKEY HK, LPCTSTR SubKey);

BOOL GetRegAnyPtr(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD *Type,
                  BYTE *RetVal, DWORD *nBytes);
BOOL GetRegAnyAlloc(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD *Type,
                    BYTE **RetVal, DWORD *nBytes);
BOOL GetRegAny(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Type,
               BYTE *RetVal, DWORD *nBytes);
BOOL SetRegAny(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Type, BYTE *Data,
               DWORD nBytes, BOOL bFlush = FALSE);

BOOL DelRegKey(HKEY hKey, LPTSTR pszSubKey);
BOOL DelRegKeyChildren(HKEY hKey, LPTSTR pszSubKey);
BOOL RegDelVal(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, BOOL bFlush = FALSE);

BOOL RegRenameVal(HKEY HK, LPCTSTR SubKey, LPCTSTR OldName, LPCTSTR NewName);

BOOL SetRegInt(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Value);
DWORD GetRegInt(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Default);

__int64 GetRegInt64(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, __int64 Default);
BOOL SetRegInt64(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, __int64 Value);

BOOL SetRegStr(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, LPCTSTR Value);
BOOL GetRegStr(HKEY HK, LPCTSTR SubKey, LPCTSTR Val, LPTSTR Str, DWORD ccMax);

BOOL RegEnum(HKEY HK, LPCTSTR SubKey, int Index, LPTSTR Str, DWORD ccMax);
BOOL RegEnumValName(HKEY HK, LPTSTR SubKey, int Index, LPTSTR Str, DWORD ccMax);

BOOL RenameRegKey(HKEY hKeyRoot, LPTSTR sSrc, LPTSTR sDst);

DWORD hKeyToKeyName(HKEY key, LPTSTR KeyName, DWORD nBytes);

#define EmptyPWAllowed                                                         \
  (GetRegInt(HKLM, _T("SYSTEM\\CurrentControlSet\\Control\\Lsa"),              \
             _T("limitblankpassworduse"), 1) == 0)

#define AllowEmptyPW(b)                                                        \
  SetRegInt(HKLM, _T("SYSTEM\\CurrentControlSet\\Control\\Lsa"),               \
            _T("limitblankpassworduse"), (b) == 0)

int inline SafeMsgBox(HWND w, LPTSTR m, LPTSTR c, DWORD f) {
  if (GetModuleHandle(TEXT("Shell32.dll")) == 0)
    LoadLibrary(TEXT("Shell32.dll"));
  return MessageBox(w, m, c, f | MB_SYSTEMMODAL);
};

// Privilege stuff
BOOL DisablePrivilege(HANDLE hToken, LPCTSTR name);
BOOL EnablePrivilege(HANDLE hToken, LPCTSTR name,
                     DWORD how = SE_PRIVILEGE_ENABLED);
BOOL EnablePrivilege(LPCTSTR name);

//  AllowAccess
void AllowAccess(HANDLE hObject);
void AllowAccess(LPTSTR FileName);
void SetRegistryTreeAccess(LPTSTR KeyName, LPTSTR Account, bool bAllow);
void SetRegistryTreeAccess(LPTSTR KeyName, DWORD Rid, bool bAllow);
BOOL HasRegistryKeyAccess(LPTSTR KeyName, LPTSTR Account);
BOOL HasRegistryKeyAccess(LPTSTR KeyName, DWORD Rid);

// SetAdminDenyUserAccess
PACL SetAdminDenyUserAccess(PACL pOldDACL, PSID UserSID,
                            DWORD Permissions = SYNCHRONIZE);
void SetAdminDenyUserAccess(HANDLE hObject, PSID UserSID,
                            DWORD Permissions = SYNCHRONIZE);
void SetAdminDenyUserAccess(HANDLE hObject, DWORD ProcessID = 0,
                            DWORD Permissions = SYNCHRONIZE);

// GetUserAccessSD
PSECURITY_DESCRIPTOR GetUserAccessSD();

//  NetworkPathToUNCPath
BOOL NetworkPathToUNCPath(LPTSTR path);

// ResolveCommandLine: Beautify path, try to resolve executables (msi,cpl...)
BOOL ResolveCommandLine(IN LPWSTR CmdLine, IN LPCWSTR CurDir, OUT LPTSTR cmd);

// Link Creation
BOOL CreateLink(LPCTSTR fname, LPCTSTR lnk_fname, int iIcon);

// DeleteDirectory ... all files and SubDirs
bool DeleteDirectory(LPCTSTR DIR);

// UserName:
bool GetSIDUserName(PSID sid, LPTSTR User, LPTSTR Domain = 0);
PSID GetTokenUserSID(HANDLE hToken);
PSID GetProcessUserSID(DWORD ProcessID);
bool GetTokenUserName(HANDLE hUser, LPTSTR User, LPTSTR Domain = 0);
bool GetProcessUserName(DWORD ProcessID, LPTSTR User, LPTSTR Domain = 0);

// GetProcessID
DWORD GetProcessID(LPCTSTR ProcName, DWORD SessID = -1);
// GetProcessName
BOOL GetProcessName(DWORD PID, LPTSTR ProcessName);

// GetProcessUserToken
HANDLE GetProcessUserToken(DWORD ProcId);
// Shell stuff
HANDLE GetShellProcessToken();

// GetTokenGroups
PTOKEN_GROUPS GetTokenGroups(HANDLE hToken);

// GetLogonSid
PSID GetLogonSid(HANDLE hToken);

// UserIsInSuRunnersOrAdmins
DWORD UserIsInSuRunnersOrAdmins(HANDLE hToken);
DWORD UserIsInSuRunnersOrAdmins();

//  GetShellPID
DWORD GetShellPID(DWORD SessID);

//  GetSessionUserToken
HANDLE GetSessionUserToken(DWORD SessID);

//  GetSessionLogonSID
PSID GetSessionLogonSID(DWORD SessionID);

//  GetProcessFileName
DWORD GetModuleFileNameAEx(HMODULE hMod, LPSTR lpFilename, DWORD nSize);
DWORD GetModuleFileNameWEx(HMODULE hMod, LPWSTR lpFilename, DWORD nSize);
DWORD GetProcessFileName(LPWSTR lpFilename, DWORD nSize);

DWORD GetParentProcessID(DWORD PID);
// BOOL SetParentProcessID(HANDLE hProcess,DWORD PID);
BOOL SetProcessUserToken(HANDLE hProcess, HANDLE hUser);
// ASLR
inline LPVOID BASE_ADDR(LPVOID LocAddr, HANDLE hProcess) {
  LPVOID p = LocAddr;
  DWORD d;
  HMODULE hMod = 0;
  if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &d))
    p = (LPVOID)((BYTE *)LocAddr - (BYTE *)GetModuleHandle(0) + (BYTE *)hMod);
  //  else
  //    DBGTrace1("EnumProcessModules failed",GetLastErrorNameStatic());
  //  DBGTrace2("BASE_ADDR(%p)==%p",LocAddr,p);
  return p;
}

// GetVersionString
LPCTSTR GetVersionString();

// LoadUserBitmap
HBITMAP LoadUserBitmap(LPCTSTR UserName);

// GetMenuShieldIcon
HBITMAP GetMenuShieldIcon();

// CTimeOut
class CTimeOut {
public:
  CTimeOut();
  CTimeOut(DWORD TimeOut);
  void Set(DWORD TimeOut);
  DWORD Rest();
  bool TimedOut();

protected:
  DWORD m_EndTime;
};

// CImpersonateSessionUser
class CImpersonateSessionUser {
public:
  CImpersonateSessionUser(DWORD SessionID) {
    m_Token = 0;
    if (SessionID != -1) {
      m_Token = GetSessionUserToken(SessionID);
      if (m_Token) {
        if (!ImpersonateLoggedOnUser(m_Token)) {
          CloseHandle(m_Token);
          m_Token = 0;
        }
      }
    }
  }
  ~CImpersonateSessionUser() {
    if (m_Token) {
      RevertToSelf();
      CloseHandle(m_Token);
    }
  }

protected:
  HANDLE m_Token;
};

// CImpersonateProcessUser
class CImpersonateProcessUser {
public:
  CImpersonateProcessUser(DWORD ProcessID) {
    m_Token = 0;
    if (ProcessID)
      m_Token = GetProcessUserToken(ProcessID);
    if (m_Token) {
      if (!ImpersonateLoggedOnUser(m_Token)) {
        CloseHandle(m_Token);
        m_Token = 0;
      }
    }
  }
  ~CImpersonateProcessUser() {
    if (m_Token) {
      RevertToSelf();
      CloseHandle(m_Token);
    }
  }

protected:
  HANDLE m_Token;
};

// strwldcmp returns true if s matches pattern case insensitive
// pattern may contain '*' and '?' as wildcards
// '?' any "one" character in s match
// '*' any "zero or more" characters in s match
// e.G. strwldcmp("Test me","t*S*") strwldcmp("Test me","t?S*e") would match
bool strwldcmp(LPCTSTR s, LPCTSTR pattern);

/////////////////////////////////////////////////////////////////////////////
//
// Show Window on primary Monitor
//
/////////////////////////////////////////////////////////////////////////////
void BringToPrimaryMonitor(HWND hWnd);

void SR_PathStripPathA(LPSTR p);
void SR_PathStripPathW(LPWSTR p);
void SR_PathQuoteSpacesW(LPWSTR p);
LPTSTR SR_PathGetArgsW(LPCWSTR p);
BOOL SR_PathAppendW(LPWSTR p, LPCWSTR a);
