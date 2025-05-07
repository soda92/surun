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
#include <Aclapi.h>
#include <MMSYSTEM.H>
#include <SHLWAPI.H>
#include <Sddl.h>
#include <ShlGuid.h>
#include <Shobjidl.h>
#include <Tlhelp32.h>
#include <lm.h>
#include <tchar.h>
#include "DynWTSAPI.h"

#include "DBGTRace.h"
#include "Helpers.h"
#include "IsAdmin.h"
#include "ResStr.h"
#include "SuRunVer.h"
#include "UserGroups.h"
#include "lsa_laar.h"
#include "resource.h"
#include <algorithm>
using std::max;
using std::min;

#pragma comment(lib, "ShlWapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Mpr.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "WINMM.LIB")

#if _MSC_VER >= 1500
unsigned int _osplatform = 0;
unsigned int _osver = 0;
unsigned int _winmajor = 0;
unsigned int _winminor = 0;

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
unsigned int _a_winver = _GetWinVer();
#endif //_MSC_VER

LANGID SetLocale(LANGID locale) {

  if ((_winmajor > 5)) {
    typedef LANGID(WINAPI * FMSetThreadUILanguage)(LANGID);
    FMSetThreadUILanguage fnPtr = (FMSetThreadUILanguage)GetProcAddress(
        GetModuleHandle(_T("kernel32.dll")), "SetThreadUILanguage");
    if (fnPtr)
      return (*fnPtr)(locale);
  }
  return ::SetThreadLocale(MAKELCID(locale, SORT_DEFAULT));
}

//////////////////////////////////////////////////////////////////////////////
//
//  Registry Helper
//
//////////////////////////////////////////////////////////////////////////////
PACL CopyAcl(PACL pFrom) {
  if (!pFrom)
    return NULL;
  ACL_SIZE_INFORMATION info;
  if (!GetAclInformation(pFrom, &info, sizeof(info), AclSizeInformation))
    return NULL;
  PACL pTo = (PACL)malloc(info.AclBytesInUse);
  if (!pTo)
    return NULL;
  if (!InitializeAcl(pTo, info.AclBytesInUse, ACL_REVISION)) {
    free(pTo);
    return NULL;
  }
  for (DWORD i = 0; i < info.AceCount; i++) {
    ACE_HEADER *pace = 0;
    if (GetAce(pFrom, i, (void **)(&pace)))
      AddAce(pTo, ACL_REVISION, MAXDWORD, pace, pace->AceSize);
  }
  return pTo;
}

bool RegSetDACL(HKEY HK, LPCTSTR SubKey, PACL pACL) {
  CBigResStr sRegKey(
      L"%s\\%s",
      (HK == HKLM)
          ? (L"MACHINE")
          : ((HK == HKCU)
                 ? (L"CURRENT_USER")
                 : ((HK == HKCR) ? (L"CLASSES_ROOT")
                                 : ((HK == HKEY_USERS) ? (L"USERS") : (L"")))),
      SubKey);
  DWORD e =
      SetNamedSecurityInfo(sRegKey, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION,
                           NULL, NULL, pACL, NULL);
  if (e != ERROR_SUCCESS)
    DBGTrace2("SetNamedSecurityInfo(%s) error: %s", sRegKey,
              GetErrorNameStatic(e));
  return e == ERROR_SUCCESS;
}

PACL RegGrantAdminAccess(HKEY HK, LPCTSTR SubKey) {
  CBigResStr sRegKey(
      L"%s\\%s",
      (HK == HKLM)
          ? (L"MACHINE")
          : ((HK == HKCU)
                 ? (L"CURRENT_USER")
                 : ((HK == HKCR) ? (L"CLASSES_ROOT")
                                 : ((HK == HKEY_USERS) ? (L"USERS") : (L"")))),
      SubKey);
  PSECURITY_DESCRIPTOR pSD = NULL;
  PACL pOldDACL = NULL;
  PACL pNewDACL = NULL;
  PSID pOldOwner = NULL;
  PSID pNewOwner = NULL;
  DWORD e = GetNamedSecurityInfo(sRegKey, SE_REGISTRY_KEY,
                                 OWNER_SECURITY_INFORMATION |
                                     DACL_SECURITY_INFORMATION,
                                 &pOldOwner, NULL, &pOldDACL, NULL, &pSD);
  if (e == ERROR_SUCCESS) {
    // Initialize Admin SID
    EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
    EnablePrivilege(SE_RESTORE_NAME);
    SID_IDENTIFIER_AUTHORITY AdminSidAuth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&AdminSidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &pNewOwner)) {
      // Take ownership
      e = SetNamedSecurityInfo(sRegKey, SE_REGISTRY_KEY,
                               OWNER_SECURITY_INFORMATION, pNewOwner, NULL,
                               NULL, NULL);
      if (e == ERROR_SUCCESS) {
        // Initialize an EXPLICIT_ACCESS structure for an ACE.
        // The ACE will allow Admins full access to the key.
        EXPLICIT_ACCESS ea = {KEY_ALL_ACCESS,
                              SET_ACCESS,
                              SUB_CONTAINERS_AND_OBJECTS_INHERIT,
                              {0, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_SID,
                               TRUSTEE_IS_WELL_KNOWN_GROUP, (LPTSTR)pNewOwner}};
        // Create a new ACL that merges the new ACE into the existing DACL.
        e = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
        if (e == ERROR_SUCCESS) {
          // Set DACL
          e = SetNamedSecurityInfo(sRegKey, SE_REGISTRY_KEY,
                                   DACL_SECURITY_INFORMATION, NULL, NULL,
                                   pNewDACL, NULL);
          if (e != ERROR_SUCCESS)
            DBGTrace1("SetNamedSecurityInfo error: %s", GetErrorNameStatic(e));
        } else
          DBGTrace1("SetEntriesInAcl error: %s", GetErrorNameStatic(e));
        // release ownership
        e = SetNamedSecurityInfo(sRegKey, SE_REGISTRY_KEY,
                                 OWNER_SECURITY_INFORMATION, pOldOwner, NULL,
                                 NULL, NULL);
        if (e != ERROR_SUCCESS) {
          TCHAR u[2 * DNLEN];
          GetSIDUserName(pOldOwner, u);
          DBGTrace2("SetNamedSecurityInfo(Owner=%s) error: %s", u,
                    GetErrorNameStatic(e));
        }
      } else
        DBGTrace1("SetNamedSecurityInfo error: %s", GetErrorNameStatic(e));
    } else
      DBGTrace1("AllocateAndInitializeSid error: %s", GetLastErrorNameStatic());
  } else
    DBGTrace1("GetNamedSecurityInfo error: %s", GetErrorNameStatic(e));
  if (pNewDACL)
    LocalFree((HLOCAL)pNewDACL);
  if (pNewOwner)
    FreeSid(pNewOwner);
  if (pOldDACL)
    pOldDACL = CopyAcl(pOldDACL);
  if (pSD)
    LocalFree((HLOCAL)pSD);
  return pOldDACL;
}

BOOL GetRegAny(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Type,
               BYTE *RetVal, DWORD *nBytes) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_READ), &Key);
  if (dwRes == ERROR_SUCCESS) {
    DWORD dwType = Type;
    BOOL bRet = (RegQueryValueEx(Key, ValName, NULL, &dwType, RetVal, nBytes) ==
                 ERROR_SUCCESS) &&
                (dwType == Type);
    RegCloseKey(Key);
    return bRet;
  }
  return FALSE;
}

BOOL GetRegAnyPtr(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD *Type,
                  BYTE *RetVal, DWORD *nBytes) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_READ), &Key);
  if (dwRes == ERROR_SUCCESS) {
    BOOL bRet = RegQueryValueEx(Key, ValName, NULL, Type, RetVal, nBytes) ==
                ERROR_SUCCESS;
    RegCloseKey(Key);
    return bRet;
  }
  return FALSE;
}

BOOL GetRegAnyAlloc(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD *Type,
                    BYTE **RetVal, DWORD *nBytes) {
  HKEY Key;
  BOOL bRet = FALSE;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_READ), &Key);
  if (dwRes == ERROR_SUCCESS) {
    if ((RegQueryValueEx(Key, ValName, NULL, Type, 0, nBytes) ==
         ERROR_SUCCESS) &&
        (*nBytes)) {
      *RetVal = (BYTE *)malloc(*nBytes);
      if (*RetVal)
        bRet = RegQueryValueEx(Key, ValName, NULL, Type, *RetVal, nBytes) ==
               ERROR_SUCCESS;
    }
    RegCloseKey(Key);
  }
  return bRet;
}

BOOL SetRegAny(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Type, BYTE *Data,
               DWORD nBytes, BOOL bFlush /*=FALSE*/) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_WRITE), &Key);
  if ((dwRes != ERROR_SUCCESS) && (dwRes != ERROR_ACCESS_DENIED))
    dwRes = RegCreateKeyEx(HK, SubKey, 0, 0, 0, KSAM(KEY_WRITE), 0, &Key, 0);
  if (dwRes == ERROR_SUCCESS) {
    LONG l = RegSetValueEx(Key, ValName, 0, Type, Data, nBytes);
    if (bFlush)
      RegFlushKey(Key);
    RegCloseKey(Key);
    return l == ERROR_SUCCESS;
  } else if (dwRes != ERROR_ACCESS_DENIED)
    DBGTrace3("RegCreateKeyEx(%x,%s) failed: %s", HK, SubKey,
              GetErrorNameStatic(dwRes));
  else
    DBGTrace3("RegOpenKeyEx(%x,%s) failed: %s", HK, SubKey,
              GetErrorNameStatic(dwRes));
  return FALSE;
}

BOOL RegDelVal(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName,
               BOOL bFlush /*=FALSE*/) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_WRITE), &Key);
  if (dwRes == ERROR_SUCCESS) {
    BOOL bRet = RegDeleteValue(Key, ValName) == ERROR_SUCCESS;
    if (bFlush)
      RegFlushKey(Key);
    RegCloseKey(Key);
    return bRet;
  } else
    DBGTrace3("RegOpenKeyEx(%x,%s) failed: %s", HK, SubKey,
              GetErrorNameStatic(dwRes));
  return FALSE;
}

BOOL RegRenameVal(HKEY HK, LPCTSTR SubKey, LPCTSTR OldName, LPCTSTR NewName) {
  BYTE *v = 0;
  DWORD n = 0;
  DWORD t = 0;
  BOOL bOk = GetRegAnyAlloc(HK, SubKey, OldName, &t, &v, &n);
  if (bOk) {
    bOk = SetRegAny(HK, SubKey, NewName, t, v, n);
    if (bOk)
      bOk = RegDelVal(HK, SubKey, OldName);
    free(v);
  }
  return bOk;
}

DWORD GetRegInt(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Default) {
  DWORD RetVal = Default;
  DWORD n = sizeof(RetVal);
  if (GetRegAny(HK, SubKey, ValName, REG_DWORD, (BYTE *)&RetVal, &n))
    return RetVal;
  return Default;
}

BOOL SetRegInt(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, DWORD Value) {
  return SetRegAny(HK, SubKey, ValName, REG_DWORD, (BYTE *)&Value,
                   sizeof(DWORD));
}

__int64 GetRegInt64(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, __int64 Default) {
  __int64 RetVal = Default;
  DWORD n = sizeof(RetVal);
  if (GetRegAny(HK, SubKey, ValName, REG_BINARY, (BYTE *)&RetVal, &n))
    return RetVal;
  return Default;
}

BOOL SetRegInt64(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, __int64 Value) {
  return SetRegAny(HK, SubKey, ValName, REG_BINARY, (BYTE *)&Value,
                   sizeof(__int64));
}

BOOL GetRegStr(HKEY HK, LPCTSTR SubKey, LPCTSTR Val, LPTSTR Str, DWORD ccMax) {
  if (GetRegAny(HK, SubKey, Val, REG_SZ, (BYTE *)Str, &ccMax))
    return true;
  TCHAR s[4096];
  DWORD n = 4096;
  if (GetRegAny(HK, SubKey, Val, REG_EXPAND_SZ, (BYTE *)&s, &n))
    return ExpandEnvironmentStrings(s, Str, min(4096, (int)ccMax)), TRUE;
  return FALSE;
}

BOOL SetRegStr(HKEY HK, LPCTSTR SubKey, LPCTSTR ValName, LPCTSTR Value) {
  return SetRegAny(HK, SubKey, ValName, REG_SZ, (BYTE *)Value,
                   (DWORD)_tcslen(Value) * sizeof(TCHAR));
}

BOOL RegEnum(HKEY HK, LPCTSTR SubKey, int Index, LPTSTR Str, DWORD ccMax) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_READ), &Key);
  if (dwRes == ERROR_SUCCESS) {
    BOOL bRet = (RegEnumKey(Key, Index, Str, ccMax) == ERROR_SUCCESS);
    RegCloseKey(Key);
    return bRet;
  }
  return false;
}

BOOL RegEnumValName(HKEY HK, LPTSTR SubKey, int Index, LPTSTR Str,
                    DWORD ccMax) {
  HKEY Key;
  DWORD dwRes = RegOpenKeyEx(HK, SubKey, 0, KSAM(KEY_READ), &Key);
  if (dwRes == ERROR_SUCCESS) {
    BOOL bRet =
        (RegEnumValue(Key, Index, Str, &ccMax, 0, 0, 0, 0) == ERROR_SUCCESS);
    RegCloseKey(Key);
    return bRet;
  }
  return false;
}

BOOL DelRegKey(HKEY hKey, LPTSTR pszSubKey) {
  HKEY hEnumKey;
  if (RegOpenKeyEx(hKey, pszSubKey, 0, KSAM(KEY_ENUMERATE_SUB_KEYS),
                   &hEnumKey) != NOERROR)
    return FALSE;
  TCHAR szKey[4096];
  DWORD dwSize = 4096;
  while (ERROR_SUCCESS ==
         RegEnumKeyEx(hEnumKey, 0, szKey, &dwSize, 0, 0, 0, 0)) {
    DelRegKey(hEnumKey, szKey);
    dwSize = 4096;
  }
  RegCloseKey(hEnumKey);
  RegDeleteKey(hKey, pszSubKey);
  return TRUE;
}

BOOL DelRegKeyChildren(HKEY hKey, LPTSTR pszSubKey) {
  HKEY hEnumKey;
  if (RegOpenKeyEx(hKey, pszSubKey, 0, KSAM(KEY_ENUMERATE_SUB_KEYS),
                   &hEnumKey) != NOERROR)
    return FALSE;
  TCHAR szKey[4096];
  DWORD dwSize = 4096;
  while (ERROR_SUCCESS ==
         RegEnumKeyEx(hEnumKey, 0, szKey, &dwSize, 0, 0, 0, 0)) {
    DelRegKey(hEnumKey, szKey);
    dwSize = 4096;
  }
  RegCloseKey(hEnumKey);
  return TRUE;
}

void CopyRegKey(HKEY hSrc, HKEY hDst) {
  LPTSTR s = (LPTSTR)malloc(512 * sizeof(TCHAR));
  BYTE *b = (BYTE *)malloc(8192);
  DWORD i, nS, nB, t;
  for (i = 0, nS = 512, nB = 8192;
       0 == RegEnumValue(hSrc, i, s, &nS, 0, &t, b, &nB);
       nS = 512, nB = 8192, i++)
    RegSetValueEx(hDst, s, 0, t, b, nB);
  free(b);
  for (i = 0, nS = 512; 0 == RegEnumKey(hSrc, i, s, nS); i++) {
    HKEY newDst, newSrc;
    if (0 == RegOpenKeyEx(hSrc, s, 0, KSAM(KEY_ALL_ACCESS), &newSrc)) {
      if (0 == RegCreateKeyEx(hDst, s, 0, 0, 0, KSAM(KEY_ALL_ACCESS), 0,
                              &newDst, 0)) {
        CopyRegKey(newSrc, newDst);
        RegCloseKey(newDst);
      }
      RegCloseKey(newSrc);
    }
  }
  free(s);
}

BOOL RenameRegKey(HKEY hKeyRoot, LPTSTR sSrc, LPTSTR sDst) {
  HKEY hSrc, hDst;
  if (RegOpenKeyEx(hKeyRoot, sSrc, 0, KSAM(KEY_ALL_ACCESS), &hSrc))
    return FALSE;
  if (RegCreateKeyEx(hKeyRoot, sDst, 0, 0, 0, KSAM(KEY_ALL_ACCESS), 0, &hDst,
                     0))
    return RegCloseKey(hSrc), FALSE;
  CopyRegKey(hSrc, hDst);
  RegCloseKey(hDst);
  RegCloseKey(hSrc);
  DelRegKey(hKeyRoot, sSrc);
  return TRUE;
}

DWORD hKeyToKeyName(HKEY key, LPTSTR KeyName, DWORD nBytes) {
  if (key && (nBytes > 4)) {
    typedef DWORD(__stdcall * zwQK)(HANDLE, int, PVOID, ULONG, PULONG);
    static zwQK g_zwqk = 0;
    if (!g_zwqk)
      g_zwqk = reinterpret_cast<zwQK>(
          GetProcAddress(GetModuleHandle(L"ntdll.dll"), "ZwQueryKey"));
    if (g_zwqk) {
      DWORD n;
      if (g_zwqk(key, 3, KeyName, nBytes - 4, &n) == ERROR_SUCCESS) {
        memmove(KeyName, &((BYTE *)KeyName)[4], n - 4);
        ((BYTE *)KeyName)[n - 4] = 0;
        return n - 4;
      }
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Privilege stuff
//
//////////////////////////////////////////////////////////////////////////////

BOOL DisablePrivilege(HANDLE hToken, LPCTSTR name) {
  return EnablePrivilege(hToken, name, 0);
}

BOOL EnablePrivilege(HANDLE hToken, LPCTSTR name,
                     DWORD how /*=SE_PRIVILEGE_ENABLED*/) {
  TOKEN_PRIVILEGES priv = {1, {0, 0, how}};
  LookupPrivilegeValue(0, name, &priv.Privileges[0].Luid);
  AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(priv), 0, 0);
  return GetLastError() == ERROR_SUCCESS;
}

BOOL EnablePrivilege(LPCTSTR name) {
  HANDLE hToken = 0;
  BOOL bRet = TRUE;
  if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, 0,
                       &hToken)) {
    bRet &= (GetLastError() == ERROR_NO_TOKEN);
    if (!bRet) {
      DBGTrace1(
          "EnablePrivilege(%s) OpenThreadToken(GetCurrentThread()) failed",
          name);
    }
  } else {
    bRet &= EnablePrivilege(hToken, name);
    if (!bRet) {
      DBGTrace1("EnablePrivilege(%s) on Thread Token failed", name);
    }
    CloseHandle(hToken);
  }
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
                        &hToken)) {
    {
      DBGTrace1(
          "EnablePrivilege(%s) OpenProcessToken(GetCurrentProcess()) failed",
          name);
    }
    return FALSE;
  }
  bRet &= EnablePrivilege(hToken, name);
  CloseHandle(hToken);
  //   if (!bRet)
  //   {
  //     DBGTrace1("EnablePrivilege(%s) failed", name);
  //   }
  return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  AllowAccess
//
//////////////////////////////////////////////////////////////////////////////

void AllowAccess(HANDLE hObject) {
  DWORD dwRes;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea;
  SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
  PSID pEveryoneSID = NULL;
  if (NULL == hObject)
    return;
  // Get a pointer to the existing DACL.
  dwRes = GetSecurityInfo(hObject, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                          NULL, NULL, &pOldDACL, NULL, &pSD);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("GetSecurityInfo failed: %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Create a well-known SID for the Everyone group.
  if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0,
                                0, 0, 0, 0, &pEveryoneSID)) {
    DBGTrace1("AllocateAndInitializeSid failed: %s", GetLastErrorNameStatic());
    return;
  }
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  // The ACE will allow Everyone read access to the key.
  memset(&ea, 0, sizeof(EXPLICIT_ACCESS));
  ea.grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
  ea.grfAccessMode = GRANT_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;
  // Create a new ACL that merges the new ACE
  // into the existing DACL.
  dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetEntriesInAcl failed: %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Attach the new ACL as the object's DACL.
  dwRes = SetSecurityInfo(hObject, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                          NULL, NULL, pNewDACL, NULL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetSecurityInfo failed: %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
Cleanup:
  if (pSD != NULL)
    LocalFree((HLOCAL)pSD);
  if (pNewDACL != NULL)
    LocalFree((HLOCAL)pNewDACL);
  if (pEveryoneSID)
    FreeSid(pEveryoneSID);
}

void AllowAccess(LPTSTR FileName) {
  DWORD dwRes;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea;
  SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
  PSID pEveryoneSID = NULL;
  if (NULL == FileName)
    return;
  // Get a pointer to the existing DACL.
  dwRes =
      GetNamedSecurityInfo(FileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                           NULL, NULL, &pOldDACL, NULL, &pSD);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("GetNamedSecurityInfo Error %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Create a well-known SID for the Everyone group.
  if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0,
                                0, 0, 0, 0, &pEveryoneSID)) {
    DBGTrace1("AllocateAndInitializeSid Error %s", GetLastErrorNameStatic());
    return;
  }
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  // The ACE will allow Users read access to the key.
  memset(&ea, 0, sizeof(EXPLICIT_ACCESS));
  ea.grfAccessPermissions = FILE_ALL_ACCESS;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;
  // Create a new ACL that merges the new ACE
  // into the existing DACL.
  dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetEntriesInAcl Error %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Attach the new ACL as the object's DACL.
  dwRes =
      SetNamedSecurityInfo(FileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                           NULL, NULL, pNewDACL, NULL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetNamedSecurityInfo Error %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
Cleanup:
  if (pSD != NULL)
    LocalFree((HLOCAL)pSD);
  if (pNewDACL != NULL)
    LocalFree((HLOCAL)pNewDACL);
  if (pEveryoneSID)
    FreeSid(pEveryoneSID);
}

void SetRegistryTreeAccess(LPTSTR KeyName, LPTSTR Account, bool bAllow) {
  DWORD dwRes;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea = {0};
  if (NULL == KeyName)
    return;
  // Get a pointer to the existing DACL.
  dwRes =
      GetNamedSecurityInfo(KeyName, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION,
                           NULL, NULL, &pOldDACL, NULL, &pSD);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("GetNamedSecurityInfo failed %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  BuildExplicitAccessWithName(&ea, Account, KEY_ALL_ACCESS,
                              bAllow ? SET_ACCESS : REVOKE_ACCESS,
                              SUB_CONTAINERS_AND_OBJECTS_INHERIT);
  // Create a new ACL that merges the new ACE into the existing DACL.
  dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetEntriesInAcl failed %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  // Attach the new ACL as the object's DACL.
  dwRes =
      SetNamedSecurityInfo(KeyName, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION,
                           NULL, NULL, pNewDACL, NULL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetNamedSecurityInfo failed %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
Cleanup:
  if (pSD != NULL)
    LocalFree((HLOCAL)pSD);
  if (pNewDACL != NULL)
    LocalFree((HLOCAL)pNewDACL);
}

BOOL HasRegistryKeyAccess(LPTSTR KeyName, LPTSTR Account) {
  if (NULL == KeyName)
    return 0;
  PSID AccountSID = GetAccountSID(Account);
  if (AccountSID == NULL)
    return false;
  DWORD dwRes;
  PEXPLICIT_ACCESS pea = NULL;
  DWORD n = 0;
  {
    PACL pDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    // Get a pointer to the existing DACL.
    dwRes = GetNamedSecurityInfo(KeyName, SE_REGISTRY_KEY,
                                 DACL_SECURITY_INFORMATION, NULL, NULL, &pDACL,
                                 NULL, &pSD);
    if (ERROR_SUCCESS != dwRes) {
      DBGTrace3(
          "HasRegistryKeyAccess(\"%s\", \"%s\") GetNamedSecurityInfo failed %s",
          KeyName, Account, GetErrorNameStatic(dwRes));
      if (pSD != NULL)
        LocalFree((HLOCAL)pSD);
      free(AccountSID);
      return false;
    }
    dwRes = GetExplicitEntriesFromAcl(pDACL, &n, &pea);
    if (pSD != NULL)
      LocalFree((HLOCAL)pSD);
    if (ERROR_SUCCESS != dwRes) {
      DBGTrace3("HasRegistryKeyAccess(\"%s\", \"%s\") "
                "GetExplicitEntriesFromAcl failed %s",
                KeyName, Account, GetErrorNameStatic(dwRes));
      free(AccountSID);
      return false;
    }
  }
  for (DWORD i = 0; i < n; i++) {
    if ((pea[i].grfAccessMode == GRANT_ACCESS) &&
        ((pea[i].grfAccessPermissions & KEY_WRITE) == KEY_WRITE) &&
        (pea[i].Trustee.TrusteeForm == TRUSTEE_IS_SID)) {
      if (EqualSid(AccountSID, pea[i].Trustee.ptstrName)) {
        free(AccountSID);
        LocalFree(pea);
        return true;
      }
    }
  }
  free(AccountSID);
  if (pea != NULL)
    LocalFree(pea);
  return false;
}

BOOL HasRegistryKeyAccess(LPTSTR KeyName, DWORD Rid) {
  DWORD cchG = GNLEN;
  WCHAR Group[GNLEN + 1] = {0};
  if (GetGroupName(Rid, Group, &cchG))
    return HasRegistryKeyAccess(KeyName, Group);
  return false;
}

void SetRegistryTreeAccess(LPTSTR KeyName, DWORD Rid, bool bAllow) {
  DWORD cchG = GNLEN;
  WCHAR Group[GNLEN + 1] = {0};
  if (GetGroupName(Rid, Group, &cchG))
    SetRegistryTreeAccess(KeyName, Group, bAllow);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetAdminDenyUserAccess
//
//////////////////////////////////////////////////////////////////////////////
PACL SetAdminDenyUserAccess(PACL pOldDACL, PSID UserSID,
                            DWORD Permissions /*=SYNCHRONIZE*/) {
  SID_IDENTIFIER_AUTHORITY AdminSidAuthority = SECURITY_NT_AUTHORITY;
  PSID AdminSID = NULL;
  // Initialize Admin SID
  if (!AllocateAndInitializeSid(
          &AdminSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
          DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminSID)) {
    DBGTrace1("AllocateAndInitializeSid failed %s", GetLastErrorNameStatic());
    return 0;
  }
  // Initialize EXPLICIT_ACCESS structures
  EXPLICIT_ACCESS ea[2] = {0};
  // The ACE will deny the current User access to the object.
  ea[0].grfAccessPermissions = Permissions;
  ea[0].grfAccessMode = SET_ACCESS;
  ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[0].Trustee.ptstrName = (LPTSTR)UserSID;
  // The ACE will allow Administrators full access to the object.
  ea[1].grfAccessPermissions = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
  ea[1].grfAccessMode = GRANT_ACCESS;
  ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[1].Trustee.ptstrName = (LPTSTR)AdminSID;
  // Create a new ACL that merges the new ACE
  // into the existing DACL.
  PACL pNewDACL = NULL;
  DWORD dwRes = SetEntriesInAcl(2, &ea[0], pOldDACL, &pNewDACL);
  if (ERROR_SUCCESS != dwRes)
    DBGTrace1("SetEntriesInAcl failed: %s", GetErrorNameStatic(dwRes));
  if (AdminSID)
    FreeSid(AdminSID);
  return pNewDACL;
}

void SetAdminDenyUserAccess(HANDLE hObject, PSID UserSID,
                            DWORD Permissions /*=SYNCHRONIZE*/) {
  if (NULL == hObject) {
    DBGTrace("SetAdminDenyUserAccess failed(No Object)!");
    return;
  }
  PACL pOldDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  // Get a pointer to the existing DACL.
  DWORD dwRes =
      GetSecurityInfo(hObject, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                      NULL, NULL, &pOldDACL, NULL, &pSD);
  if (ERROR_SUCCESS == dwRes) {
    PACL pNewDACL = SetAdminDenyUserAccess(pOldDACL, UserSID, Permissions);
    // Attach the new ACL as the object's DACL.
    if (pNewDACL) {
      dwRes =
          SetSecurityInfo(hObject, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                          NULL, NULL, pNewDACL, NULL);
      if (ERROR_SUCCESS != dwRes)
        DBGTrace1("SetSecurityInfo failed: %s", GetErrorNameStatic(dwRes));
      LocalFree((HLOCAL)pNewDACL);
    } else
      DBGTrace("SetAdminDenyUserAccess failed!");
  } else
    DBGTrace1("GetSecurityInfo failed: %s", GetErrorNameStatic(dwRes));
  if (pSD != NULL)
    LocalFree((HLOCAL)pSD);
}

void SetAdminDenyUserAccess(HANDLE hObject, DWORD ProcessID /*=0*/,
                            DWORD Permissions /*=SYNCHRONIZE*/) {
  if (ProcessID == 0)
    ProcessID = GetCurrentProcessId();
  PSID UserSID = GetProcessUserSID(ProcessID);
  if (!UserSID)
    return;
  SetAdminDenyUserAccess(hObject, UserSID, Permissions);
  free(UserSID);
}

//////////////////////////////////////////////////////////////////////////////
//
// GetUserAccessSD:
//   create a self relative "full access" Security Descriptor for the current
//   user
//
//////////////////////////////////////////////////////////////////////////////

PSECURITY_DESCRIPTOR GetUserAccessSD() {
  PSID pUserSID = GetProcessUserSID(GetCurrentProcessId());
  PACL pACL = NULL;
  PSECURITY_DESCRIPTOR pSD = 0;
  PSECURITY_DESCRIPTOR pSDret = 0;
  EXPLICIT_ACCESS ea = {0};
  DWORD SDlen = 0;
  ea.grfAccessPermissions =
      STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL | GENERIC_READ | GENERIC_WRITE;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea.Trustee.ptstrName = (LPTSTR)pUserSID;
  DWORD dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
  if (ERROR_SUCCESS != dwRes) {
    DBGTrace1("SetEntriesInAcl failed: %s", GetErrorNameStatic(dwRes));
    goto Cleanup;
  }
  pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
  if (pSD == NULL) {
    DBGTrace1("LocalAlloc failed: %s", GetLastErrorNameStatic());
    goto Cleanup;
  }
  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
    DBGTrace1("InitializeSecurityDescriptor failed: %s",
              GetLastErrorNameStatic());
    goto Cleanup;
  }
  if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
    DBGTrace1("SetSecurityDescriptorDacl failed: %s", GetLastErrorNameStatic());
    goto Cleanup;
  }
  MakeSelfRelativeSD(pSD, pSDret, &SDlen);
  pSDret = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SDlen);
  if (!MakeSelfRelativeSD(pSD, pSDret, &SDlen)) {
    DBGTrace1("MakeSelfRelativeSD failed: %s", GetLastErrorNameStatic());
    goto Cleanup;
  }
  free(pUserSID);
  LocalFree(pACL);
  LocalFree(pSD);
  return pSDret;
Cleanup:
  if (pUserSID)
    free(pUserSID);
  if (pACL)
    LocalFree(pACL);
  if (pSD)
    LocalFree(pSD);
  if (pSDret)
    LocalFree(pSDret);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
//  NetworkPathToUNCPath
//
//  This will convert a path on a mapped network share to a UNC path
//  e.g. if you mapped \\Server\C$ to your drive Z:\ this function will
//  convert Z:\Sound\MOD4WIN\TheBest to \\Server\C$\Sound\MOD4WIN\TheBest
//
//  This conversion is required since you lose network connections when
//  logging on as different user.
//
//  NOTE1: The newly logged on user needs to have access permission to the
//         network share on the server
//  NOTE2: This function will fail if path is a "remembered" connection
//////////////////////////////////////////////////////////////////////////////

BOOL NetworkPathToUNCPath(LPTSTR path) {
  if ((_tcslen(path) > 1) && (path[0] == '\\') && (path[1] == '\\'))
    return true;
  if (!PathIsNetworkPath(path))
    return FALSE;
  DWORD cb = 4096;
  TCHAR UNCPath[4096] = {0};
  UNIVERSAL_NAME_INFO uni;
  uni.lpUniversalName = &UNCPath[0];
  UNIVERSAL_NAME_INFO *puni = (UNIVERSAL_NAME_INFO *)&UNCPath;
  DWORD dwErr =
      WNetGetUniversalName(path, UNIVERSAL_NAME_INFO_LEVEL, puni, &cb);
  if (dwErr != NO_ERROR) {
    DBGTrace2("WNetGetUniversalName(%s) failed: %s", path,
              GetErrorNameStatic(dwErr));
    return FALSE;
  }
  _tcscpy(path, puni->lpUniversalName);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// QualifyPath
//
//////////////////////////////////////////////////////////////////////////////

// Combine path parts
void Combine(LPTSTR Dst, LPTSTR path, LPTSTR file, LPTSTR ext) {
  _tcscpy(Dst, path);
  CHK_BOOL_FN(PathAppend(Dst, file));
  _tcscat(Dst, ext);
}

// Split path parts
void Split(LPTSTR app, LPTSTR path, LPTSTR file, LPTSTR ext) {
  // Get Path
  _tcscpy(path, app);
  PathRemoveFileSpec(path);
  // Get File, Ext
  _tcscpy(file, app);
  PathStripPath(file);
  _tcscpy(ext, PathFindExtension(file));
  PathRemoveExtension(file);
}

BOOL QualifyPath(LPTSTR app, LPTSTR path, LPTSTR file, LPTSTR ext,
                 LPCTSTR CurDir) {
  static LPTSTR ExeExts[] = {L".exe", L".lnk", L".cmd",
                             L".bat", L".com", L".pif"};
  if (path[0] == '.') {
    // relative path: make it absolute
    _tcscpy(app, CurDir);
    CHK_BOOL_FN(PathAppend(app, path));
    CHK_BOOL_FN(PathCanonicalize(path, app));
    Combine(app, path, file, ext);
  }
  if (path[0] == '\\') {
    if (path[1] == '\\')
      // UNC path: must be fully qualified!
      return PathFileExists(app) && (!PathIsDirectory(app));
    // Root of current drive
    _tcscpy(path, CurDir);
    CHK_BOOL_FN(PathStripToRoot(path));
    Combine(app, path, file, ext);
  }
  if (path[0] == 0) {
    _tcscpy(path, app);
    LPCTSTR d[2] = {CurDir, 0};
    // file.ext ->search in current dir and %path%
    if ((PathFindOnPath(path, d)) && (!PathIsDirectory(path))) {
      // Done!
      _tcscpy(app, path);
      CHK_BOOL_FN(PathRemoveFileSpec(path));
      return TRUE;
    }
    // Not found! Try all Extensions for Executables
    if (ext[0] == 0)
      for (int i = 0; i < countof(ExeExts); i++)
      // file ->search (exe,bat,cmd,com,pif,lnk) in current dir, search %path%
      {
        _stprintf(path, L"%s%s", file, ExeExts[i]);
        if ((PathFindOnPath(path, d)) && (!PathIsDirectory(path))) {
          // Done!
          _tcscpy(app, path);
          _tcscpy(ext, ExeExts[i]);
          CHK_BOOL_FN(PathRemoveFileSpec(path));
          return TRUE;
        }
      }
    // Search AppPaths:
    if (GetRegStr(HKCU, CBigResStr(L"%s\\%s", APP_PATHS, app), 0, path, 4096)) {
      // Found!
      if (!GetRegStr(HKCU, CBigResStr(L"%s\\%s", APP_PATHS, app), L"Path", path,
                     4096)) {
        _tcscpy(app, path);
        CHK_BOOL_FN(PathRemoveFileSpec(path));
      }
      return TRUE;
    }
    if (GetRegStr(HKLM, CBigResStr(L"%s\\%s", APP_PATHS, app), 0, path, 4096)) {
      // Found!
      if (!GetRegStr(HKLM, CBigResStr(L"%s\\%s", APP_PATHS, app), L"Path", path,
                     4096)) {
        _tcscpy(app, path);
        CHK_BOOL_FN(PathRemoveFileSpec(path));
      }
      return TRUE;
    }
    // Not found! Try all Extensions for Executables
    if (ext[0] == 0)
      for (int i = 0; i < countof(ExeExts); i++) {
        if (GetRegStr(HKCU,
                      CBigResStr(L"%s\\%s", APP_PATHS,
                                 CBigResStr(L"%s%s", app, ExeExts[i])),
                      0, path, 4096)) {
          // Found!
          if (!GetRegStr(HKCU,
                         CBigResStr(L"%s\\%s", APP_PATHS,
                                    CBigResStr(L"%s%s", app, ExeExts[i])),
                         L"Path", path, 4096)) {
            _tcscpy(app, path);
            CHK_BOOL_FN(PathRemoveFileSpec(path));
          }
          _tcscpy(ext, ExeExts[i]);
          return TRUE;
        }
        if (GetRegStr(HKLM,
                      CBigResStr(L"%s\\%s", APP_PATHS,
                                 CBigResStr(L"%s%s", app, ExeExts[i])),
                      0, path, 4096)) {
          // Found!
          if (!GetRegStr(HKLM,
                         CBigResStr(L"%s\\%s", APP_PATHS,
                                    CBigResStr(L"%s%s", app, ExeExts[i])),
                         L"Path", path, 4096)) {
            _tcscpy(app, path);
            CHK_BOOL_FN(PathRemoveFileSpec(path));
          }
          _tcscpy(ext, ExeExts[i]);
          return TRUE;
        }
      }
    return FALSE;
  }
  if (path[1] == ':') {
    static TCHAR d[4096];
    zero(d);
    GetCurrentDirectory(countof(d), d);
    // if path=="d:" -> "cd d:"
    if (!SetCurrentDirectory(path)) {
      DBGTrace2("SetCurrentDirectory(%s) failed: %s", path,
                GetLastErrorNameStatic());
      return false;
    }
    // if path=="d:" -> "cd d:" -> "d:\documents"
    GetCurrentDirectory(4096, path);
    CHK_BOOL_FN(SetCurrentDirectory(d));
    Combine(app, path, file, ext);
  }
  // d:\path\file.ext ->PathFileExists
  if (PathFileExists(app) && (!PathIsDirectory(app)))
    return TRUE;
  if (ext[0] == 0)
    for (int i = 0; i < countof(ExeExts); i++)
    // Not found! Try all Extensions for Executables
    //  file ->search (exe,bat,cmd,com,pif,lnk) in path
    {
      Combine(app, path, file, ExeExts[i]);
      if ((PathFileExists(app)) && (!PathIsDirectory(app))) {
        // Done!
        _tcscpy(ext, ExeExts[i]);
        return TRUE;
      }
    }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// ResolveCommandLine: Based on SuDown (http://SuDown.sourceforge.net)
//
//////////////////////////////////////////////////////////////////////////////

BOOL ResolveCommandLine(IN LPWSTR CmdLine, IN LPCWSTR CurDir, OUT LPTSTR cmd) {
  // ToDo: use dynamic allocated strings
  if (((CmdLine == NULL) || ((*CmdLine) == 0)) &&
      ((cmd == NULL) || ((*cmd) == 0)))
    return false;
  if (StrLenW(CmdLine) + StrLenW(CurDir) > 4096 - 64)
    return false;
  // Check for URLs...
  {
    LPCWSTR dsp = wcsstr(CmdLine, L"://");
    if (dsp) {
      // Url after first space is ok
      LPCWSTR sp = wcsstr(CmdLine, L" ");
      if (!sp) // No spaces, fail!
        return false;
      if (sp > dsp) // first space after url, fail!
        return false;
    }
  }
  // Application
  static TCHAR app[4096];
  zero(app);
  _tcscpy(app, CmdLine);
  PathRemoveBlanks(app);
  // Command line parameters
  static TCHAR args[4096] = {0};
  zero(args);
  _tcscpy(args, PathGetArgs(app));
  PathRemoveBlanks(args);
  PathRemoveArgs(app);
  PathUnquoteSpaces(app);
  NetworkPathToUNCPath(app);
  BOOL fExist = PathFileExists(app);
  // Split path parts
  static TCHAR path[4096];
  static TCHAR file[4096 + 1];
  static TCHAR ext[4096 + 1];
  static TCHAR SysDir[4096 + 1];
  zero(path);
  zero(file);
  zero(ext);
  zero(SysDir);
  GetSystemDirectory(SysDir, 4096);
  Split(app, path, file, ext);
  // Explorer(.exe)
  if ((!fExist) && (!_wcsicmp(app, L"explorer")) ||
      (!_wcsicmp(app, L"explorer.exe"))) {
    if (args[0] == 0)
      _tcscpy(args, L"/e, C:");
    GetSystemWindowsDirectory(app, 4096);
    PathAppend(app, L"explorer.exe");
  } else
    // Msconfig(.exe) is not in path but found by windows
    if ((!fExist) && (!_wcsicmp(app, L"msconfig")) ||
        (!_wcsicmp(app, L"msconfig.exe"))) {
      GetSystemWindowsDirectory(app, 4096);
      PathAppend(app, L"pchealth\\helpctr\\binaries\\msconfig.exe");
      if (!PathFileExists(app))
        _tcscpy(app, L"msconfig");
    } else
      // Control Panel special folder files:
      if ((args[0] == 0) &&
          ((!fExist) && ((!_wcsicmp(app, L"control.exe")) ||
                         (!_wcsicmp(app, L"control"))) ||
           (fExist && (!_wcsicmp(path, SysDir)) &&
            (!_wcsicmp(file, L"control")) && (!_wcsicmp(ext, L".exe"))))) {
        GetSystemWindowsDirectory(app, 4096);
        PathAppend(app, L"explorer.exe");
        if (_winmajor < 6)
          // 2k/XP: Control Panel is beneath "my computer"!
          _tcscpy(args, L"/n, "
                        L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{"
                        L"21EC2020-3AEA-1069-A2DD-08002B30309D}");
        else
          // Vista: Control Panel is beneath desktop!
          _tcscpy(args, L"/n, ::{21EC2020-3AEA-1069-A2DD-08002B30309D}");
      } else if (((!_wcsicmp(app, L"ncpa.cpl")) && (args[0] == 0)) ||
                 (fExist && (!_wcsicmp(path, SysDir)) &&
                  (!_wcsicmp(file, L"ncpa")) && (!_wcsicmp(ext, L".cpl")))) {
        GetSystemWindowsDirectory(app, 4096);
        PathAppend(app, L"explorer.exe");
        if (_winmajor < 6)
          // 2k/XP: Control Panel is beneath "my computer"!
          _tcscpy(args, L"/n, "
                        L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{"
                        L"21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{7007ACC7-"
                        L"3202-11D1-AAD2-00805FC1270E}");
        else
          // Vista: Control Panel is beneath desktop!
          _tcscpy(args, L"/n, "
                        L"::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{"
                        L"7007ACC7-3202-11D1-AAD2-00805FC1270E}");
      } else
        //*.reg files
        if (!_wcsicmp(ext, L".reg")) {
          PathQuoteSpaces(app);
          _tcscpy(args, app);
          GetSystemWindowsDirectory(app, 4096);
          PathAppend(app, L"regedit.exe");
        } else
          // Control Panel files
          if (!_wcsicmp(ext, L".cpl")) {
            PathQuoteSpaces(app);
            if (args[0] && app[0])
              wcscat(app, L",");
            wcscat(app, args);
            _tcscpy(args, L"shell32.dll,Control_RunDLLAsUser ");
            wcscat(args, app);
            GetSystemDirectory(app, 4096);
            PathAppend(app, L"rundll32.exe");
          } else
            // Windows Installer files
            if (!_wcsicmp(ext, L".msi")) {
              PathQuoteSpaces(app);
              if (args[0] && app[0]) {
                wcscat(app, L" ");
                wcscat(app, args);
                _tcscpy(args, app);
              } else {
                _tcscpy(args, L"/i ");
                wcscat(args, app);
              }
              GetSystemDirectory(app, 4096);
              PathAppend(app, L"msiexec.exe");
            } else
              // Windows Management Console Sanp-In
              if (!_wcsicmp(ext, L".msc")) {
                if (path[0] == 0) {
                  GetSystemDirectory(path, 4096);
                  PathAppend(path, app);
                  _tcscpy(app, path);
                  zero(path);
                }
                PathQuoteSpaces(app);
                if (args[0] && app[0])
                  wcscat(app, L" ");
                wcscat(app, args);
                _tcscpy(args, app);
                GetSystemDirectory(app, 4096);
                PathAppend(app, L"mmc.exe");
              } else
                // Try to fully qualify the executable:
                if (!QualifyPath(app, path, file, ext, CurDir)) {
                  _tcscpy(app, CmdLine);
                  PathRemoveArgs(app);
                  PathUnquoteSpaces(app);
                  NetworkPathToUNCPath(app);
                  Split(app, path, file, ext);
                }
  _tcscpy(cmd, app);
  fExist = PathFileExists(app);
  PathQuoteSpaces(cmd);
  if (args[0] && app[0])
    wcscat(cmd, L" ");
  wcscat(cmd, args);
  return fExist;
}

//////////////////////////////////////////////////////////////////////////////
//
// CreateLink, creates a LNK file
//
//////////////////////////////////////////////////////////////////////////////

BOOL CreateLink(LPCTSTR fname, LPCTSTR lnk_fname, int iIcon) {
  // Save parameters
  TCHAR args[4096] = {0};
  _tcscpy(args, PathGetArgs(fname));
  // Application
  TCHAR app[4096] = {0};
  _tcscpy(app, fname);
  PathRemoveArgs(app);
  PathUnquoteSpaces(app);
  NetworkPathToUNCPath(app);
  // Get Path
  TCHAR path[4096];
  _tcscpy(path, app);
  PathRemoveFileSpec(path);
  if (!PathFileExists(app))
    return false;
  BOOL bRes = FALSE;
  IShellLink *psl = NULL;
  IPersistFile *pPf = NULL;
  if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                              IID_IShellLink, (LPVOID *)&psl)))
    goto cleanup;
  if (FAILED(psl->QueryInterface(IID_IPersistFile, (LPVOID *)&pPf)))
    goto cleanup;
  if (FAILED(psl->SetPath(app)))
    goto cleanup;
  if (FAILED(psl->SetWorkingDirectory(path)))
    goto cleanup;
  if (FAILED(psl->SetIconLocation(app, iIcon)))
    goto cleanup;
  if (args[0]) {
    if (FAILED(psl->SetArguments(args)))
      goto cleanup;
  }
  if (FAILED(pPf->Save(lnk_fname, TRUE)))
    goto cleanup;
  bRes = TRUE;
cleanup:
  if (pPf)
    pPf->Release();
  if (psl)
    psl->Release();
  return bRes;
}

//////////////////////////////////////////////////////////////////////////////
//
// DeleteDirectory ... all files and SubDirs
//
//////////////////////////////////////////////////////////////////////////////

bool DeleteDirectory(LPCTSTR DIR) {
  bool bRet = true;
  WIN32_FIND_DATA fd = {0};
  TCHAR s[4096];
  _tcscpy(s, DIR);
  PathAppend(s, _T("*.*"));
  HANDLE hFind = FindFirstFile(s, &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      _tcscpy(s, DIR);
      PathAppend(s, fd.cFileName);
      if (PathIsDirectory(s)) {
        if (_tcscmp(fd.cFileName, _T(".")) && _tcscmp(fd.cFileName, _T(".."))) {
          SetFileAttributes(s, FILE_ATTRIBUTE_DIRECTORY);
          bRet = bRet && DeleteDirectory(s);
        }
      } else {
        SetFileAttributes(s, FILE_ATTRIBUTE_NORMAL);
        bRet = bRet && DeleteFile(s);
      }
    } while (FindNextFile(hFind, &fd));
  }
  FindClose(hFind);
  bRet = bRet && RemoveDirectory(DIR);
  return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetTokenUserName
//
//////////////////////////////////////////////////////////////////////////////

bool GetSIDUserName(PSID sid, LPTSTR User, LPTSTR Domain /*=0*/) {
  SID_NAME_USE snu;
  TCHAR uName[UNLEN + 1], dName[UNLEN + 1];
  DWORD uLen = UNLEN, dLen = UNLEN;
  if (!LookupAccountSid(NULL, sid, uName, &uLen, dName, &dLen, &snu)) {
    DBGTrace1("LookupAccountSid failed: %s", GetLastErrorNameStatic());
    return FALSE;
  }
  if (Domain == 0) {
    _tcscpy(User, dName);
    if (_tcslen(User))
      _tcscat(User, TEXT("\\"));
    _tcscat(User, uName);
  } else {
    _tcscpy(User, uName);
    _tcscpy(Domain, dName);
  }
  return TRUE;
}

bool GetTokenUserName(HANDLE hUser, LPTSTR User, LPTSTR Domain /*=0*/) {
  DWORD dwLen = 0;
  if ((!GetTokenInformation(hUser, TokenUser, NULL, 0, &dwLen)) &&
      (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
    DBGTrace1("GetTokenInformation failed: %s", GetLastErrorNameStatic());
    return false;
  }
  TOKEN_USER *ptu = (TOKEN_USER *)malloc(dwLen);
  if (!ptu) {
    DBGTrace("malloc failed");
    return false;
  }
  if (GetTokenInformation(hUser, TokenUser, (PVOID)ptu, dwLen, &dwLen))
    GetSIDUserName(ptu->User.Sid, User, Domain);
  else
    DBGTrace1("GetTokenInformation failed: %s", GetLastErrorNameStatic());
  free(ptu);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetTokenUserSID
//
//////////////////////////////////////////////////////////////////////////////

PSID GetTokenUserSID(HANDLE hToken) {
  PSID sid = 0;
  DWORD dwLen = 0;
  if ((GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen)) ||
      (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
    TOKEN_USER *ptu = (TOKEN_USER *)malloc(dwLen);
    if (ptu) {
      if (GetTokenInformation(hToken, TokenUser, (PVOID)ptu, dwLen, &dwLen)) {
        dwLen = GetLengthSid(ptu->User.Sid);
        sid = (PSID)malloc(dwLen);
        if (sid)
          CopySid(dwLen, sid, ptu->User.Sid);
        else
          DBGTrace("malloc failed");
      } else
        DBGTrace1("GetTokenInformation failed: %s", GetLastErrorNameStatic());
      free(ptu);
    }
  } else
    DBGTrace1("GetTokenInformation failed: %s", GetLastErrorNameStatic());
  return sid;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetProcessUserSID
//
//////////////////////////////////////////////////////////////////////////////

PSID GetProcessUserSID(DWORD ProcessID) {
  EnablePrivilege(SE_DEBUG_NAME);
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessID);
  if (!hProc) {
    DBGTrace2("OpenProcess(%d) failed: %s", ProcessID,
              GetLastErrorNameStatic());
    return 0;
  }
  HANDLE hToken;
  PSID sid = 0;
  // Open impersonation token for process
  if (OpenProcessToken(hProc, TOKEN_QUERY, &hToken)) {
    sid = GetTokenUserSID(hToken);
    CloseHandle(hToken);
  } else
    DBGTrace2("OpenProcessToken(ID==%d) failed: %s", ProcessID,
              GetLastErrorNameStatic());
  CloseHandle(hProc);
  return sid;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetProcessUserName
//
//////////////////////////////////////////////////////////////////////////////

bool GetProcessUserName(DWORD ProcessID, LPTSTR User, LPTSTR Domain /*=0*/) {
  EnablePrivilege(SE_DEBUG_NAME);
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessID);
  if (!hProc) {
    DBGTrace2("OpenProcess(%d) failed: %s", ProcessID,
              GetLastErrorNameStatic());
    return 0;
  }
  HANDLE hToken;
  bool bRet = false;
  // Open impersonation token for process
  if (OpenProcessToken(hProc, TOKEN_QUERY, &hToken)) {
    bRet = GetTokenUserName(hToken, User, Domain);
    CloseHandle(hToken);
  } else
    DBGTrace2("OpenProcessToken(ID==%d) failed: %s", ProcessID,
              GetLastErrorNameStatic());
  CloseHandle(hProc);
  return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetProcessID
//
//  Get the Process ID for a file name. Filename is stripped to the bones e.g.
//  "Explorer.exe" instead of "C:\Windows\Explorer.exe"
//  If there are multiple "Explorer.exe"s running in your Logon session, the
//  function will return the ID of the first Process it finds.
//
//  The purpose of this function ist primarily to get the Process ID of the
//  Shell process.
//////////////////////////////////////////////////////////////////////////////

DWORD GetProcessID(LPCTSTR ProcName, DWORD SessID /*=-1*/) {
  if (SessID != (DWORD)-1) {
    // Terminal Services:
    DWORD nProcesses = 0;
    WTS_PROCESS_INFO *pwtspi = 0;
    WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pwtspi,
                          &nProcesses);
    for (DWORD Process = 0; Process < nProcesses; Process++) {
      if (pwtspi[Process].SessionId != SessID)
        continue;
      TCHAR PName[MAX_PATH] = {0};
      _tcscpy(PName, pwtspi[Process].pProcessName);
      PathStripPath(PName);
      if (_tcsicmp(ProcName, PName) == 0) {
        return WTSFreeMemory(pwtspi), pwtspi[Process].ProcessId;
      }
    }
    WTSFreeMemory(pwtspi);
  }
  // ToolHelp
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE) {
    DBGTrace1("CreateToolhelp32Snapshot failed: %s", GetLastErrorNameStatic());
    return 0;
  }
  DWORD dwRet = 0;
  PROCESSENTRY32 pe = {0};
  pe.dwSize = sizeof(PROCESSENTRY32);
  bool bFirst = true;
  while ((bFirst ? Process32First(hSnap, &pe) : Process32Next(hSnap, &pe))) {
    bFirst = false;
    PathStripPath(pe.szExeFile);
    if (_tcsicmp(ProcName, pe.szExeFile) != 0)
      continue;
    if ((SessID != (DWORD)-1)) {
      ULONG s = (DWORD)-2;
      if ((!ProcessIdToSessionId(pe.th32ProcessID, &s)) || (s != SessID)) {
        DBGTrace1("ProcessIdToSessionId failed: %s", GetLastErrorNameStatic());
        continue;
      }
    }
    dwRet = pe.th32ProcessID;
    break;
  }
  CloseHandle(hSnap);
  if (dwRet == 0)
    DBGTrace1("GetProcessID: %s not found", ProcName);
  return dwRet;
}

BOOL GetProcessName(DWORD PID, LPTSTR ProcessName) {
  // Terminal Services:
  DWORD nProcesses = 0;
  WTS_PROCESS_INFO *pwtspi = 0;
  WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pwtspi, &nProcesses);
  for (DWORD Process = 0; Process < nProcesses; Process++) {
    if (pwtspi[Process].ProcessId == PID) {
      _tcscpy(ProcessName, pwtspi[Process].pProcessName);
      // PathStripPath(ProcessName);
      return WTSFreeMemory(pwtspi), TRUE;
    }
  }
  WTSFreeMemory(pwtspi);
  // ToolHelp
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE) {
    DBGTrace1("CreateToolhelp32Snapshot failed: %s", GetLastErrorNameStatic());
    return FALSE;
  }
  BOOL bRet = FALSE;
  PROCESSENTRY32 pe = {0};
  pe.dwSize = sizeof(PROCESSENTRY32);
  bool bFirst = true;
  while ((bFirst ? Process32First(hSnap, &pe) : Process32Next(hSnap, &pe))) {
    bFirst = false;
    if (pe.th32ProcessID == PID) {
      _tcscpy(ProcessName, pe.szExeFile);
      // PathStripPath(ProcessName);
      bRet = TRUE;
      break;
    }
  }
  CloseHandle(hSnap);
  return bRet;
}

/////////////////////////////////////////////////////////////////////////////
//
// GetShellProcessToken
//
/////////////////////////////////////////////////////////////////////////////

HANDLE GetShellProcessToken() {
  DWORD ShellID = 0;
  GetWindowThreadProcessId(GetShellWindow(), &ShellID);
  if (!ShellID) {
    DBGTrace2("GetWindowThreadProcessId(0x%X) failed: %s", GetShellWindow(),
              GetLastErrorNameStatic());
    return 0;
  }
  HANDLE hShell = OpenProcess(PROCESS_QUERY_INFORMATION, 0, ShellID);
  if (!hShell) {
    DBGTrace2("OpenProcess(%d) failed: %s", ShellID, GetLastErrorNameStatic());
    return 0;
  }
  HANDLE hTok = 0;
  if (!OpenProcessToken(hShell, TOKEN_DUPLICATE, &hTok)) {
    DBGTrace2("OpenProcessToken(%d) failed: %s", ShellID,
              GetLastErrorNameStatic());
  }
  CloseHandle(hShell);
  return hTok;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetTokenGroups
//
//////////////////////////////////////////////////////////////////////////////

PTOKEN_GROUPS GetTokenGroups(HANDLE hToken) {
  DWORD cbBuffer = 0;
  PTOKEN_GROUPS ptgGroups = NULL;
  GetTokenInformation(hToken, TokenGroups, NULL, cbBuffer, &cbBuffer);
  if (cbBuffer) {
    ptgGroups = (PTOKEN_GROUPS)malloc(cbBuffer);
    if (ptgGroups) {
      CHK_BOOL_FN(GetTokenInformation(hToken, TokenGroups, ptgGroups, cbBuffer,
                                      &cbBuffer));
    } else
      DBGTrace("malloc failed");
  } else
    DBGTrace1("GetTokenInformation failed: %s", GetLastErrorNameStatic());
  return ptgGroups;
}

//////////////////////////////////////////////////////////////////////////////
//
// FindLogonSID
//
//////////////////////////////////////////////////////////////////////////////

PSID FindLogonSID(PTOKEN_GROUPS ptg) {
  for (UINT i = 0; i < ptg->GroupCount; i++)
    if (((ptg->Groups[i].Attributes & SE_GROUP_LOGON_ID) ==
         SE_GROUP_LOGON_ID) &&
        (IsValidSid(ptg->Groups[i].Sid)))
      return ptg->Groups[i].Sid;
  DBGTrace("FindLogonSID: No Logon SID found!");
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetLogonSid ...copies the SID from FindLogonSID to a new buffer
//
//////////////////////////////////////////////////////////////////////////////

PSID GetLogonSid(HANDLE hToken) {
  PTOKEN_GROUPS ptgGroups = GetTokenGroups(hToken);
  if (!ptgGroups)
    return 0;
  PSID sid = FindLogonSID(ptgGroups);
  if (sid) {
    DWORD dwSidLength = GetLengthSid(sid);
    PSID pLogonSid = (PSID)malloc(dwSidLength);
    if (pLogonSid) {
      if (CopySid(dwSidLength, pLogonSid, sid)) {
        free(ptgGroups);
        return pLogonSid;
      }
      free(pLogonSid);
    }
  }
  free(ptgGroups);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// UserIsInSuRunnersOrAdmins
//
//////////////////////////////////////////////////////////////////////////////

DWORD UserIsInSuRunnersOrAdmins(HANDLE hToken) {
  PTOKEN_GROUPS ptg = GetTokenGroups(hToken);
  DWORD dwRet = 0;
  if (IsSplitAdmin(hToken))
    dwRet |= IS_SPLIT_ADMIN;
  if (GetSystemMetrics(SM_REMOTESESSION))
    dwRet |= IS_TERMINAL_USER;
  if (!ptg)
    return 0;
  SID_IDENTIFIER_AUTHORITY AdminSidAuthority = SECURITY_NT_AUTHORITY;
  PSID AdminSID = NULL;
  // Initialize Admin SID
  if (!AllocateAndInitializeSid(
          &AdminSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
          DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminSID)) {
    DBGTrace1("AllocateAndInitializeSid failed: %s", GetLastErrorNameStatic());
    return free(ptg), 0;
  }
  PSID SuRunnersSID = GetAccountSID(SURUNNERSGROUP);
  for (UINT i = 0; i < ptg->GroupCount; i++)
    if ((ptg->Groups[i].Attributes &
         (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT |
          SE_GROUP_MANDATORY)) &&
        (IsValidSid(ptg->Groups[i].Sid))) {
      if (EqualSid(ptg->Groups[i].Sid, AdminSID))
        dwRet |= IS_IN_ADMINS;
      else if (SuRunnersSID && EqualSid(ptg->Groups[i].Sid, SuRunnersSID))
        dwRet |= IS_IN_SURUNNERS;
    }
  FreeSid(AdminSID);
  free(SuRunnersSID);
  free(ptg);
  return dwRet;
}

DWORD UserIsInSuRunnersOrAdmins() {
  HANDLE hToken = NULL;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    DBGTrace1("OpenProcessToken failed: %s", GetLastErrorNameStatic());
    return 0;
  }
  DWORD dwRet = UserIsInSuRunnersOrAdmins(hToken);
  CloseHandle(hToken);
  return dwRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetProcessUserToken
//
//////////////////////////////////////////////////////////////////////////////
HANDLE GetProcessUserToken(DWORD ProcId) {
  EnablePrivilege(SE_DEBUG_NAME);
  HANDLE hProc = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION,
                             TRUE, ProcId);
  if (!hProc) {
    DBGTrace2("OpenProcess(%d) failed: %s", ProcId, GetLastErrorNameStatic());
    return 0;
  }
  // Open impersonation token for process
  HANDLE hToken = NULL;
  CHK_BOOL_FN(OpenProcessToken(hProc,
                               TOKEN_IMPERSONATE | TOKEN_QUERY |
                                   TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                               &hToken));
  CloseHandle(hProc);
  if (!hToken) {
    DBGTrace1("OpenProcessToken(%d) No Token", ProcId);
    return 0;
  }
  HANDLE hUser = 0;
  CHK_BOOL_FN(DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
                               SecurityIdentification, TokenPrimary, &hUser));
  return CloseHandle(hToken), hUser;
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetShellPID return the process id of the shell
//
//////////////////////////////////////////////////////////////////////////////
DWORD GetShellPID(DWORD SessID) {
  // Get the Shells Name
  TCHAR Shell[MAX_PATH];
  if (!GetRegStr(HKEY_LOCAL_MACHINE,
                 _T("SOFTWARE\\Microsoft\\Windows NT\\")
                 _T("CurrentVersion\\Winlogon"),
                 _T("Shell"), Shell, MAX_PATH))
    return 0;
  PathRemoveArgs(Shell);
  PathStripPath(Shell);
  // Now return the Shells Process ID
  return GetProcessID(Shell, SessID);
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetSessionUserToken
//
//  This function tries to use WTSQueryUserToken to get the token of the
//  currently logged on user. If the WTSQueryUserToken method fails, we'll
//  try to steal the user token of the logon sessions shell process.
//////////////////////////////////////////////////////////////////////////////

HANDLE GetSessionUserToken(DWORD SessID) {
  HANDLE hToken = NULL;
  // SE_TCB_NAME is only present in a local System User Token
  // WTSQueryUserToken requires SE_TCB_NAME!
  if ((!EnablePrivilege(SE_TCB_NAME)) ||
      (!WTSQueryUserToken(SessID, &hToken)) || (hToken == NULL)) {
    // No WTSQueryUserToken: we're in Win2k
    DWORD ShellID = GetShellPID(SessID);
    if (!ShellID)
      return 0;
    // We got the Shells Process ID, now get the user token
    return GetProcessUserToken(ShellID);
  }
  HANDLE hTokenDup = NULL;
  CHK_BOOL_FN(DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
                               SecurityIdentification, TokenPrimary,
                               &hTokenDup));
  CloseHandle(hToken);
  return hTokenDup;
}

//////////////////////////////////////////////////////////////////////////////
//
//  GetSessionLogonSID
//
//////////////////////////////////////////////////////////////////////////////

PSID GetSessionLogonSID(DWORD SessionID) {
  HANDLE hToken = GetSessionUserToken(SessionID);
  if (!hToken)
    return 0;
  PSID p = GetLogonSid(hToken);
  CloseHandle(hToken);
  return p;
}

/////////////////////////////////////////////////////////////////////////////
//
// GetProcessFileName
//
/////////////////////////////////////////////////////////////////////////////

DWORD GetModuleFileNameAEx(HMODULE hMod, LPSTR lpFilename, DWORD nSize) {
  nSize = GetModuleFileNameA(hMod, lpFilename, nSize);
  while ((lpFilename[0] == '\\') || (lpFilename[0] == '?')) {
    memmove(lpFilename, &lpFilename[1], nSize * sizeof(CHAR));
    nSize--;
  }
  return nSize;
}

DWORD GetModuleFileNameWEx(HMODULE hMod, LPWSTR lpFilename, DWORD nSize) {
  nSize = GetModuleFileNameW(hMod, lpFilename, nSize);
  while ((lpFilename[0] == L'\\') || (lpFilename[0] == L'?')) {
    memmove(lpFilename, &lpFilename[1], nSize * sizeof(WCHAR));
    nSize--;
  }
  return nSize;
}

DWORD GetProcessFileName(LPWSTR lpFilename, DWORD nSize) {
  return GetModuleFileNameWEx(0, lpFilename, nSize);
}

// NtQueryInformationProcess, NtSetInformationProcess
typedef enum {
  ProcessBasicInformation,
  ProcessQuotaLimits,
  ProcessIoCounters,
  ProcessVmCounters,
  ProcessTimes,
  ProcessBasePriority,
  ProcessRaisePriority,
  ProcessDebugPort,
  ProcessExceptionPort,
  ProcessAccessToken,
  ProcessLdtInformation,
  ProcessLdtSize,
  ProcessDefaultHardErrorMode,
  ProcessIoPortHandlers,
  ProcessPooledUsageAndLimits,
  ProcessWorkingSetWatch,
  ProcessUserModeIOPL,
  ProcessEnableAlignmentFaultFixup,
  ProcessPriorityClass,
  ProcessWx86Information,
  ProcessHandleCount,
  ProcessAffinityMask,
  ProcessPriorityBoost,
  ProcessDeviceMap,
  ProcessSessionInformation,
  ProcessForegroundInformation,
  ProcessWow64Information,
  MaxProcessInfoClass
} PROCESSINFOCLASS;

typedef struct {
  LONG ExitStatus;
  PVOID PebBaseAddress;
  ULONG_PTR AffinityMask;
  LONG BasePriority;
  ULONG_PTR UniqueProcessId;
  ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

typedef struct {
  HANDLE Token;
  HANDLE Thread;
} PROCESS_ACCESS_TOKEN;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef DWORD(CALLBACK *PROCNTSIP)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG);
PROCNTSIP NtSetInformationProcess = NULL;

typedef LONG(WINAPI *PROCNTQSIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
PROCNTQSIP NtQueryInformationProcess = NULL;

DWORD GetParentProcessID(DWORD PID) {
  if (NtQueryInformationProcess == 0)
    NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
        GetModuleHandleA("ntdll"), "NtQueryInformationProcess");
  if (NtQueryInformationProcess == 0)
    return 0;
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
  if (!hProcess)
    return 0;
  ULONG ulRetLen;
  PROCESS_BASIC_INFORMATION pbi;
  DWORD Status =
      NtQueryInformationProcess(hProcess, ProcessBasicInformation, (void *)&pbi,
                                sizeof(PROCESS_BASIC_INFORMATION), &ulRetLen);
  if (NT_SUCCESS(Status)) {
    CloseHandle(hProcess);
    return (DWORD)pbi.InheritedFromUniqueProcessId;
  }
  CloseHandle(hProcess);
  DBGTrace1("NtQueryInformationProcess failed: %s", GetErrorNameStatic(Status));
  return 0;
}

// NtSetInformationProcess(...,ProcessBasicInformation,...) Does not work
// BOOL SetParentProcessID(HANDLE hProcess,DWORD PID)
// {
//   if(NtQueryInformationProcess==0)
//     NtQueryInformationProcess =
//     (PROCNTQSIP)GetProcAddress(GetModuleHandleA("ntdll"),"NtQueryInformationProcess");
//   if(NtQueryInformationProcess==0)
//     return false;
//   if (NtSetInformationProcess==0)
//     NtSetInformationProcess=(PROCNTSIP)GetProcAddress(LoadLibraryA("ntdll.dll"),"NtSetInformationProcess");
//   if(NtSetInformationProcess==0)
//     return false;
//   ULONG ulRetLen;
//   PROCESS_BASIC_INFORMATION pbi;
//   DWORD Status=NtQueryInformationProcess(
//   hProcess,ProcessBasicInformation,(void*)&pbi,sizeof(PROCESS_BASIC_INFORMATION),&ulRetLen);
//   if (NT_SUCCESS(Status))
//   {
//     pbi.InheritedFromUniqueProcessId=PID;
//     Status=NtSetInformationProcess(hProcess,ProcessBasicInformation,(void*)&pbi,sizeof(PROCESS_BASIC_INFORMATION));
//     if (NT_SUCCESS(Status))
//       return true;
//     DBGTrace1("NtSetInformationProcess failed:
//     %s",GetErrorNameStatic(Status)); return false;
//   }
//   DBGTrace1("NtQueryInformationProcess failed:
//   %s",GetErrorNameStatic(Status)); return false;
// }

BOOL SetProcessUserToken(HANDLE hProcess, HANDLE hUser) {
  if (NtSetInformationProcess == 0)
    NtSetInformationProcess = (PROCNTSIP)GetProcAddress(
        LoadLibraryA("ntdll.dll"), "NtSetInformationProcess");
  if (NtSetInformationProcess == 0)
    return false;
  EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
  // This is highly undocumented but ReactOS/Wine/MS use it, so I do too
  PROCESS_ACCESS_TOKEN pat = {hUser, NULL};
  DWORD Status = NtSetInformationProcess(hProcess, ProcessAccessToken, &pat,
                                         sizeof(PROCESS_ACCESS_TOKEN));
  if (NT_SUCCESS(Status))
    return true;
  DBGTrace1("NtSetInformationProcess failed: %s", GetErrorNameStatic(Status));
  return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// GetVersionString
//
/////////////////////////////////////////////////////////////////////////////
LPCTSTR GetVersionString() { return LPCTSTR_VERSION; };

/////////////////////////////////////////////////////////////////////////////
//
// LoadUserBitmap
//
/////////////////////////////////////////////////////////////////////////////

DWORD LoadFile(LPCTSTR FName, BYTE **Buf, DWORD Ofs = 0, DWORD nBytes = 0) {
  if (Buf == 0)
    return 0;
  *Buf = NULL;
  HANDLE f = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0, OPEN_EXISTING, 0, 0);
  if (f == INVALID_HANDLE_VALUE)
    return 0;
  if (nBytes == 0)
    nBytes = GetFileSize(f, 0) - Ofs;
  *Buf = (BYTE *)malloc(nBytes);
  if (*Buf == NULL) {
    CloseHandle(f);
    return 0;
  }
  DWORD n = 0;
  if (Ofs)
    SetFilePointer(f, Ofs, 0, FILE_BEGIN);
  ReadFile(f, *Buf, nBytes, &n, 0);
  CloseHandle(f);
  return n;
}

HBITMAP LoadUserBitmap(LPCTSTR UserName) {
  TCHAR Pic[UNLEN + 1];
  _tcscpy(Pic, UserName);
  // DBGTrace1("LoadUserBitmap: %s",Pic);
  TCHAR PicDir[4096];
  GetRegStr(HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell ")
            _T("Folders"),
            _T("Common AppData"), PicDir, 4096);
  PathUnquoteSpaces(PicDir);
  PathAppend(PicDir, _T("Microsoft\\User Account Pictures"));
  // 1st try Domain user: "c:\ProgramData\Microsoft\User Account
  // Pictures\<Domain>+<User>.dat"
  {
    if (_tcschr(Pic, L'\\'))
      *_tcschr(Pic, L'\\') = L'+';
    HBITMAP hbm = 0;
    BYTE *bmp = NULL;
    if (LoadFile(CResStr(L"%s\\%s.dat", PicDir, Pic), &bmp)) {
      BITMAPFILEHEADER *bmfh = (BITMAPFILEHEADER *)(&bmp[16]);
      BITMAPINFO *bmi = (BITMAPINFO *)(&bmp[16 + sizeof(BITMAPFILEHEADER)]);
      void *Bits = (void *)(&bmp[16 + bmfh->bfOffBits]);
      HDC dc = GetDC(0);
      hbm = CreateDIBitmap(dc, &bmi->bmiHeader, CBM_INIT, Bits, bmi,
                           DIB_RGB_COLORS);
      ReleaseDC(0, dc);
    }
    free(bmp);
    if (hbm)
      return hbm;
  }
  _tcscpy(Pic, UserName);
  PathStripPath(Pic);
  // Vista++: Load User bitmap from registry:
  DWORD UserID = 0;
  if (GetRegAnyPtr(
          HKLM, CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\Names\\%s", Pic),
          L"", &UserID, 0, 0) &&
      UserID) {
    BYTE *bmp = 0;
    DWORD type = 0;
    DWORD nBytes = 0;
    HBITMAP hbm = 0;
    if (GetRegAnyAlloc(
            HKLM, CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\%08X", UserID),
            L"UserTile", &type, &bmp, &nBytes)) {
      BITMAPFILEHEADER *bmfh = (BITMAPFILEHEADER *)(&bmp[16]);
      BITMAPINFO *bmi = (BITMAPINFO *)(&bmp[16 + sizeof(BITMAPFILEHEADER)]);
      void *Bits = (void *)(&bmp[16 + bmfh->bfOffBits]);
      HDC dc = GetDC(0);
      hbm = CreateDIBitmap(dc, &bmi->bmiHeader, CBM_INIT, Bits, bmi,
                           DIB_RGB_COLORS);
      ReleaseDC(0, dc);
    }
    free(bmp);
    if (hbm)
      return hbm;
  }
  // User bitmap not in registry: try to load bitmap
  HBITMAP bm = (HBITMAP)LoadImage(0, CResStr(L"%s\\%s.bmp", PicDir, Pic),
                                  IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  if (bm)
    return bm;
  // no success, load default user picture
  return (HBITMAP)LoadImage(0, CResStr(L"%s\\user.bmp", PicDir), IMAGE_BITMAP,
                            0, 0, LR_LOADFROMFILE);
}

/////////////////////////////////////////////////////////////////////////////
//
// GetMenuShieldIcon
//
/////////////////////////////////////////////////////////////////////////////
HBITMAP GetMenuShieldIcon() {
  // Win2k does not support images AND text in ome menu item. Text is more
  // important! In XP the Bitmap alpha is not apparently supported
  if (_winmajor < 6)
    return NULL;
  // Getting the LUA Shield icon is a real pain when the exe is not compiled for
  // Win6++ Loadimage looks dull, ather APIs GPF sometimes
  int cx = GetSystemMetrics(SM_CXSMICON);
  int cy = GetSystemMetrics(SM_CYSMICON);
  HICON ico = NULL;
#ifndef _SR32
  HMODULE hMod = GetModuleHandle(L"SuRunExt.dll");
#else  //_SR32
  HMODULE hMod = GetModuleHandle(L"SuRunExt32.dll");
#endif //_SR32
  if (!ico)
    ico = (HICON)LoadImage(hMod, MAKEINTRESOURCE(IDI_SR_SHIELD), IMAGE_ICON, cx,
                           cy, LR_SHARED | LR_CREATEDIBSECTION);
  if (!ico)
    return NULL;
  // Copy the icon to a Bitmap
  BITMAPINFO bmi = {{sizeof(BITMAPINFOHEADER), cx, cy, 1, 32, 0,
                     static_cast<DWORD>(cx * cy * 4), 0, 0, 0, 0},
                    {{0, 0, 0, 0}}};
  void *Bits = 0;
  HDC dc = CreateCompatibleDC(NULL);
  HBITMAP hbm = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &Bits, NULL, 0);
  if (hbm) {
    SelectObject(dc, hbm);
    DrawIconEx(dc, 0, 0, ico, cx, cy, 0, NULL, DI_NORMAL);
  }
  DeleteDC(dc);
  DestroyIcon(ico);
  return hbm;
}

/////////////////////////////////////////////////////////////////////////////
//
// CTimeOut
//
/////////////////////////////////////////////////////////////////////////////

CTimeOut::CTimeOut() { Set(0); }

CTimeOut::CTimeOut(DWORD TimeOut) { Set(TimeOut); }

void CTimeOut::Set(DWORD TimeOut) { m_EndTime = timeGetTime() + TimeOut; }

DWORD CTimeOut::Rest() {
  int to = (int)m_EndTime - (int)timeGetTime();
  if (to <= 0) {
    SetLastError(ERROR_TIMEOUT);
    return 0;
  }
  return (DWORD)to;
}

bool CTimeOut::TimedOut() { return Rest() == 0; }

/////////////////////////////////////////////////////////////////////////////
//
// strwldcmp returns true if s matches pattern case insensitive
// pattern may contain '*' and '?' as wildcards
// '?' any "one" character in s match
// '*' any "zero or more" characters in s match
// e.G. strwldcmp("Test me","t*S*") strwldcmp("Test me","t?S*e") would match
//
/////////////////////////////////////////////////////////////////////////////

bool strwldcmp(LPCTSTR s, LPCTSTR pattern) {
  if ((!s) || (!pattern))
    return false;
  while (*pattern) {
    if (*s == 0)
      return (*pattern == '*') && (pattern[1] == 0);
    // ignore case; skip, if pattern is '?'
    if ((toupper(*s) == toupper(*pattern)) || (*pattern == '?')) {
      s++;
      pattern++;
    } else if (*pattern == '*') {
      pattern++;
      // nothing after '*', ok for any string
      if (*pattern == 0)
        return true;
      // pattern contains something after '*', string needs to have a match
      while (*s) {
        if (strwldcmp(s, pattern))
          return true;
        s++;
      }
      return false;
    } else
      return false;
  }
  return *s == 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// Show Window on primary Monitor
//
/////////////////////////////////////////////////////////////////////////////
void BringToPrimaryMonitor(HWND hWnd) {
  // Only top level windows
  if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
    return;
  RECT rd = {0};
  SystemParametersInfo(SPI_GETWORKAREA, 0, &rd, 0);
  RECT rw = {0};
  GetWindowRect(hWnd, &rw);
  OffsetRect(&rw, (rd.left + rd.right - rw.right - rw.left) / 2,
             (rd.top + rd.bottom - rw.bottom - rw.top) / 2);
  MoveWindow(hWnd, rw.left, rw.top, rw.right - rw.left, rw.bottom - rw.top,
             true);
}

void SR_PathStripPathA(LPSTR p) {
  // Find file name:
  LPSTR f = p;
  for (LPSTR P = p; *P; P++) {
    if (((P[0] == '\\') || (P[0] == ':') || (P[0] == '/')) && (P[1]) &&
        (P[1] != '\\') && (P[1] != '/'))
      f = P + 1;
  }
  // Copy file name to front
  if (f != p)
    memmove(p, f, strlen(f) + 1);
}

void SR_PathStripPathW(LPWSTR p) {
  // Find file name:
  LPWSTR f = p;
  for (LPWSTR P = p; *P; P++) {
    if (((P[0] == L'\\') || (P[0] == L':') || (P[0] == L'/')) && (P[1]) &&
        (P[1] != L'\\') && (P[1] != L'/'))
      f = P + 1;
  }
  // Copy file name to front
  if (f != p)
    memmove(p, f, (wcslen(f) + 1) * sizeof(WCHAR));
}

void SR_PathQuoteSpacesW(LPWSTR p) {
  if (p && wcschr(p, L' ')) {
    size_t n = wcslen(p) + 1;
    memmove(p + 1, p, n * sizeof(WCHAR));
    p[0] = L'"';
    p[n] = L'"';
    p[n + 1] = 0;
  }
}

LPTSTR SR_PathGetArgsW(LPCWSTR p) {
  if (!p)
    return 0;
  for (BOOL bInQuot = FALSE; *p; p++) {
    switch (*p) {
    case L'"':
      bInQuot = !bInQuot;
      break;
    case L' ':
      if (!bInQuot)
        return (LPTSTR)p + 1;
      break;
    }
  }
  return (LPWSTR)p;
}

BOOL SR_PathAppendW(LPWSTR p, LPCWSTR a) {
  if ((!p) || (!a))
    return FALSE;
  // Skip leading backslashes
  while (*a == L'\\')
    a++;
  if (*a == 0)
    return FALSE;
  size_t n = wcslen(p);
  if (n && (p[n - 1] != L'\\')) {
    p[n + 1] = 0;
    p[n] = L'\\';
  }
  wcscat(p, a);
  return TRUE;
}
