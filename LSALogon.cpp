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

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>
#include <lm.h>
#include <malloc.h>
#include <ntsecapi.h>
#include <shlwapi.h>
#include <tchar.h>

#include "DBGTrace.h"
#include "Helpers.h"
#include "LSA_laar.h"
#include "Resource.h"
#include "setup.h"

#include <algorithm>
using std::min;
using std::max;

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Secur32.lib ")
#pragma comment(lib, "Rpcrt4.lib")

#define AUTH_PACKAGE "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif

#ifndef SYSTEM_MANDATORY_LABEL_NO_WRITE_UP

typedef enum _TOKEN_INFORMATION_CLASS_1 {
  TokenElevationType = TokenOrigin + 1,
  TokenLinkedToken,
  TokenElevation,
  TokenHasRestrictions,
  TokenAccessInformation,
  TokenVirtualizationAllowed,
  TokenVirtualizationEnabled,
  TokenIntegrityLevel,
  TokenUIAccess,
  TokenMandatoryPolicy,
  TokenLogonSid,
} TOKEN_INFORMATION_CLASS_1,
    *PTOKEN_INFORMATION_CLASS_1;

typedef enum _TOKEN_ELEVATION_TYPE {
  TokenElevationTypeDefault = 1,
  TokenElevationTypeFull,
  TokenElevationTypeLimited,
} TOKEN_ELEVATION_TYPE,
    *PTOKEN_ELEVATION_TYPE;

typedef struct _TOKEN_LINKED_TOKEN {
  HANDLE LinkedToken;
} TOKEN_LINKED_TOKEN, *PTOKEN_LINKED_TOKEN;

#endif //SYSTEM_MANDATORY_LABEL_NO_WRITE_UP

BOOL GetElevatedToken(HANDLE &hToken) {
  if (hToken == 0) {
    DBGTrace("GetElevatedToken NULL Token");
    return FALSE;
  }
  if (_winmajor < 6) {
    DBGTrace("GetElevatedToken not supported");
    return FALSE;
  }
  // Vista++ UAC: Get the elevated token!
  TOKEN_ELEVATION_TYPE et;
  DWORD dwSize = sizeof(et);
  if (GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenElevationType,
                          &et, dwSize, &dwSize) &&
      (et == TokenElevationTypeLimited)) {
    TOKEN_LINKED_TOKEN lt = {0};
    HANDLE hAdmin = 0;
    dwSize = sizeof(lt);
    if (GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenLinkedToken,
                            &lt, dwSize, &dwSize)) {
      if (IsInAdminGroup(lt.LinkedToken) &&
          DuplicateTokenEx(lt.LinkedToken, MAXIMUM_ALLOWED, 0,
                           SecurityImpersonation, TokenPrimary, &hAdmin)) {
        CloseHandle(lt.LinkedToken);
        CloseHandle(hToken);
        hToken = hAdmin;
        return TRUE;
      }
      CloseHandle(lt.LinkedToken);
    }
  }
  DBGTrace("GetElevatedToken failed");
  return FALSE;
}

void SetHighIL(HANDLE hUser) {
  if (hUser == 0) {
    DBGTrace("SetHighIL NULL Token");
    return;
  }
  if (_winmajor < 6) {
    DBGTrace("SetHighIL not supported");
    return;
  }
  // Vista UAC: Set Integrity level!
  SID_IDENTIFIER_AUTHORITY siaMLA = {
      0, 0, 0, 0, 0, 16} /*SECURITY_MANDATORY_LABEL_AUTHORITY*/;
  PSID pSidIL = NULL;
  typedef struct _TOKEN_MANDATORY_LABEL {
    SID_AND_ATTRIBUTES Label;
  } TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;
  TOKEN_MANDATORY_LABEL TIL = {0};
  AllocateAndInitializeSid(&siaMLA, 1,
                           0x00003000L /*SECURITY_MANDATORY_HIGH_RID*/, 0, 0, 0,
                           0, 0, 0, 0, &pSidIL);
  TIL.Label.Attributes = 0x00000020 /*SE_GROUP_INTEGRITY*/;
  TIL.Label.Sid = pSidIL;
  SetTokenInformation(hUser, (TOKEN_INFORMATION_CLASS)TokenIntegrityLevel, &TIL,
                      sizeof(TOKEN_MANDATORY_LABEL));
  FreeSid(pSidIL);
}

#define LSASTR(s, S)                                                           \
  LSA_STRING s;                                                                \
  InitString(&s, S);

void InitString(PLSA_STRING LsaString, LPSTR String) {
  size_t StringLength;
  if (String == NULL) {
    LsaString->Buffer = NULL;
    LsaString->Length = 0;
    LsaString->MaximumLength = 0;
    return;
  }
  StringLength = strlen(String);
  LsaString->Buffer = String;
  LsaString->Length = (USHORT)StringLength * sizeof(CHAR);
  LsaString->MaximumLength = (USHORT)(StringLength + 1) * sizeof(CHAR);
}

int _stringLenInBytes(const LPWSTR s) {
  if (!s)
    return 0;
  return (lstrlen(s) + 1) * sizeof *s;
}

void _initUnicodeString(UNICODE_STRING *target, LPWSTR source, USHORT cbMax) {
  target->Length = max(sizeof (*source),(size_t) cbMax) - sizeof *source;
  target->MaximumLength = cbMax;
  target->Buffer = source;
}

MSV1_0_INTERACTIVE_LOGON *GetLogonRequest(LPWSTR domain, LPWSTR user,
                                          LPWSTR pass, DWORD *pcbRequest) {
  const DWORD cbHeader = sizeof(MSV1_0_INTERACTIVE_LOGON);
  const DWORD cbDom = _stringLenInBytes(domain);
  const DWORD cbUser = _stringLenInBytes(user);
  const DWORD cbPass = _stringLenInBytes(pass);
  *pcbRequest = cbHeader + cbDom + cbUser + cbPass;
  MSV1_0_INTERACTIVE_LOGON *pRequest =
      (MSV1_0_INTERACTIVE_LOGON *)malloc(*pcbRequest);
  if (!pRequest) {
    DBGTrace("malloc failed");
    return 0;
  }
  pRequest->MessageType = MsV1_0InteractiveLogon;
  char *p = (char *)(pRequest + 1); // point right past MSV1_0_INTERACTIVE_LOGON
  LPWSTR pDom = (LPWSTR)(p);
  LPWSTR pUser = (LPWSTR)(p + cbDom);
  LPWSTR pPass = (LPWSTR)(p + cbDom + cbUser);
  CopyMemory(pDom, domain, cbDom);
  CopyMemory(pUser, user, cbUser);
  CopyMemory(pPass, pass, cbPass);
  _initUnicodeString(&pRequest->LogonDomainName, pDom, (USHORT)cbDom);
  _initUnicodeString(&pRequest->UserName, pUser, (USHORT)cbUser);
  _initUnicodeString(&pRequest->Password, pPass, (USHORT)cbPass);
  return pRequest;
}

HANDLE LSALogon(DWORD SessionID, LPWSTR UserName, LPWSTR Domain,
                LPWSTR Password, bool bNoAdmin) {
  EnablePrivilege(SE_TCB_NAME);
  HANDLE hLSA;
  LSA_OPERATIONAL_MODE SecurityMode;
  LSASTR(ProcName, "Winlogon");
  if (LsaRegisterLogonProcess(&ProcName, &hLSA, &SecurityMode)) {
    DBGTrace("LsaRegisterLogonProcess failed");
    return 0;
  }
  TOKEN_SOURCE tsrc;
  QUOTA_LIMITS Quotas;
  PMSV1_0_INTERACTIVE_PROFILE *pProf = 0;
  ULONG AuthInfoSize = 0;
  PMSV1_0_INTERACTIVE_LOGON AuthInfo = 0;
  ULONG ProfLen;
  PTOKEN_GROUPS ptg = NULL;
  PSID LogonSID = NULL;
  PSID AdminSID = NULL;
  PSID LocalSid = NULL;
  USHORT cbUserName;
  USHORT cbDomain;
  USHORT cbPassword;
  HANDLE hUser = 0;
  NTSTATUS Status;
  //
  // Initialize source context structure
  //
  try {
    HANDLE hShell = GetSessionUserToken(SessionID);
    if (hShell) {
      DWORD n = 0;
      // Copy TokenSource from the Shell Process of SessionID:
      CHK_BOOL_FN(
          GetTokenInformation(hShell, TokenSource, &tsrc, sizeof(tsrc), &n));
      strcpy(tsrc.SourceName, "SuRun");
      // Copy Logon SID from the Shell Process of SessionID:
      LogonSID = GetLogonSid(hShell);
      CloseHandle(hShell);
      hShell = 0;
    } else
       throw 123;
    // Initialize Admin SID
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&sidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &AdminSID)) {
      DBGTrace1("AllocateAndInitializeSid failed %s", GetLastErrorNameStatic());
       throw 123;
    }
    // Local SID
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_LOCAL_SID_AUTHORITY;
    if (!AllocateAndInitializeSid(&IdentifierAuthority, 1, SECURITY_LOCAL_RID,
                                  0, 0, 0, 0, 0, 0, 0, &LocalSid)) {
      DBGTrace1("AllocateAndInitializeSid failed %s", GetLastErrorNameStatic());
       throw 123;
    }
    // Initialize TOKEN_GROUPS
    ptg = (PTOKEN_GROUPS)malloc(sizeof(DWORD) + 3 * sizeof(SID_AND_ATTRIBUTES));
    if (ptg == NULL) {
      DBGTrace("malloc failed");
       throw 123;
    }
    ptg->GroupCount = bNoAdmin ? 2 : 3;
    ptg->Groups[0].Sid = LogonSID;
    ptg->Groups[0].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                                SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
    ptg->Groups[1].Sid = LocalSid;
    ptg->Groups[1].Attributes =
        SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT;
    ptg->Groups[2].Sid = AdminSID;
    ptg->Groups[2].Attributes =
        SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT;
    // AuthInfo
    AuthInfo = GetLogonRequest(Domain, UserName, Password, &AuthInfoSize);
    if (!AuthInfo)
       throw 123;
    // PackageName
    LSASTR(PackageName, AUTH_PACKAGE);
    DWORD AuthPackageId = 0;
    if ((LsaLookupAuthenticationPackage(hLSA, &PackageName, &AuthPackageId)) !=
        STATUS_SUCCESS) {
      DBGTrace("LsaLookupAuthenticationPackage failed");
       throw 123;
    }
    LUID LogonLuid = {0};
    NTSTATUS SubStatus = 0;
    Status = LsaLogonUser(hLSA, &ProcName, Interactive, AuthPackageId, AuthInfo,
                          AuthInfoSize, ptg, &tsrc, (PVOID *)&pProf, &ProfLen,
                          &LogonLuid, &hUser, &Quotas, &SubStatus);
    if (Status != ERROR_SUCCESS) {
      DBGTrace("LsaLogonUser failed");
       throw 123;
    }
    if (!bNoAdmin) {
      GetElevatedToken(hUser);
      SetHighIL(hUser);
    }
  } // try
  catch(...) {} {
    cbUserName = cbDomain = cbPassword = 0; // sensitive information
    if (ptg)
      free(ptg);
    if (AuthInfo) {
      ZeroMemory(AuthInfo, AuthInfoSize); // sensitive buffer
      free(AuthInfo);
    }
    if (pProf)
      LsaFreeReturnBuffer(pProf);
    if (AdminSID)
      FreeSid(AdminSID);
    if (LocalSid)
      FreeSid(LocalSid);
    if (LogonSID)
      free(LogonSID);
  } // finally
  LsaDeregisterLogonProcess(hLSA);
  return hUser;
}

//////////////////////////////////////////////////////////////////////////////
//
// GetTempAdminToken: create a new admin, log him on, get a Token, delete the
// admin
//
//////////////////////////////////////////////////////////////////////////////
class CTokenList {
public:
  CTokenList() {
    m_UserNames = NULL;
    m_Tokens = NULL;
    m_nTokens = NULL;
  }
  HANDLE Find(LPTSTR UserName) {
    LPTSTR n = m_UserNames;
    for (DWORD i = 0; i < m_nTokens; i++, n += _tcslen(n) + 1)
      if (_tcsicmp(n, UserName) == 0)
        return m_Tokens[i];
    return NULL;
  }
  void Add(LPTSTR UserName, HANDLE Token) {
    size_t siz = m_UserNames ? _msize(m_UserNames) : 0;
    m_UserNames = (LPTSTR)realloc(
        m_UserNames, siz + sizeof(TCHAR) * (_tcslen(UserName) + 1));
    _tcscpy(&m_UserNames[siz / sizeof(TCHAR)], UserName);
    m_Tokens = (LPHANDLE)realloc(m_Tokens, (m_nTokens + 1) * sizeof(HANDLE));
    m_Tokens[m_nTokens] = Token;
    m_nTokens++;
  }
  void Remove(LPTSTR UserName) {
    LPTSTR n = m_UserNames;
    for (DWORD i = 0; i < m_nTokens; i++, n += _tcslen(n) + 1) {
      if (_tcsicmp(n, UserName) == 0) {
        HANDLE hToken = m_Tokens[i];
        if ((i + 1) < m_nTokens) {
          LPTSTR m = n + _tcslen(n) + 1;
          memmove(n, m, _msize(m_UserNames) + n - m);
          memmove(&m_Tokens[i], &m_Tokens[i + 1],
                  sizeof(HANDLE) * (m_nTokens - i));
        } else
          *n = NULL;
        m_nTokens--;
        CloseHandle(hToken);
      }
    }
  }
  void RemoveAll() {
    free(m_UserNames);
    m_UserNames = NULL;
    for (DWORD i = 0; i < m_nTokens; i++)
      CloseHandle(m_Tokens[i]);
    free(m_Tokens);
    m_Tokens = NULL;
    m_nTokens = NULL;
  }

protected:
  LPTSTR m_UserNames;
  LPHANDLE m_Tokens;
  DWORD m_nTokens;
};

static CTokenList g_Tokens;

void DeleteTempAdminToken(LPTSTR UserName) { g_Tokens.Remove(UserName); }

void DeleteTempAdminToken(HANDLE hToken) {
  TCHAR un[UNLEN + DNLEN + 2];
  GetTokenUserName(hToken, un);
  DeleteTempAdminToken(un);
}

void DeleteTempAdminTokens() { g_Tokens.RemoveAll(); }

HANDLE LogonAsAdmin(LPTSTR UserName, LPTSTR p) {
  HANDLE hUser = 0;
  TCHAR u[2 * UNLEN + 2] = {0};
  TCHAR d[2 * UNLEN + 2] = {0};
  _tcscpy(u, UserName);
  PathStripPath(u);
  _tcscpy(d, UserName);
  PathRemoveFileSpec(d);
  DWORD st = AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, UserName, 1);
  bool bWasAdmin = st == ERROR_MEMBER_IN_ALIAS;
  if ((!st) || bWasAdmin) {
    if (!LogonUser(u, d, p, LOGON32_LOGON_INTERACTIVE, 0, &hUser))
      DBGTrace3("LogonUser(%s,%s) failed: %s", (LPCTSTR)UserName, (LPCTSTR)p,
                GetLastErrorNameStatic());
    else {
      GetElevatedToken(hUser);
      SetHighIL(hUser);
    }
    if (!bWasAdmin) {
      st = AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, UserName, 0);
      if (st)
        DBGTrace2("AlterGroupMember(%s) failed %s", (LPCTSTR)UserName,
                  GetErrorNameStatic(st));
    }
  } else
    DBGTrace2("AlterGroupMember(%s) failed %s", (LPCTSTR)UserName,
              GetErrorNameStatic(st));
  return hUser;
}

HANDLE GetSavedPasswordUserToken(LPTSTR UserName) {
  TCHAR p[PWLEN] = {0};
  LoadPassword(UserName, p, PWLEN);
  if (!p[0])
    return 0;
  HANDLE hUser = LogonAsAdmin(UserName, p);
  zero(p);
  if (hUser)
    g_Tokens.Add(UserName, hUser);
  return hUser;
}

void RestoreUserPasswords() {
  BYTE *V;
  DWORD nV;
  DWORD Vtype;
  TCHAR UserID[10];
  zero(UserID);
  int i = 0;
  while (RegEnumValName(HKLM, CResStr(SVCKEY L"\\Restore"), i, UserID,
                        sizeof(UserID) / sizeof(TCHAR))) {
    V = 0;
    nV = 0;
    Vtype = 0;
    if (GetRegAnyAlloc(HKLM, CResStr(SVCKEY L"\\Restore"), UserID, &Vtype, &V,
                       &nV)) {
      if (!SetRegAny(HKLM,
                     CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\%s", UserID),
                     L"V", Vtype, V, nV, TRUE))
        DBGTrace1("Fatal Error, could not restore user(%s) credentials!",
                  UserID);
      free(V);
    } else
      DBGTrace1("Fatal Error, could not restore user(%s) credentials!", UserID);
    free(V);
    zero(UserID);
    i++;
  }
  DelRegKey(HKLM, CResStr(SVCKEY L"\\Restore"));
  RegFlushKey(HKLM);
}

HANDLE GetTempAdminToken(LPTSTR UserName) {
  HANDLE hUser = g_Tokens.Find(UserName);
  if (hUser)
    return hUser;
  CTimeLog l(L"GetTempAdminToken(%s)", UserName);
  DWORD UserID = 0;
  TCHAR u[UNLEN + 1];
  _tcscpy(u, UserName);
  PathStripPath(u);
  // This will only work on local accounts!
  if (GetStoreUsrPW(UserName) ||
      (!GetRegAnyPtr(
          HKLM, CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\Names\\%s", u),
          L"", &UserID, 0, 0)) ||
      (UserID == 0)) {
    // Domain users will have to enter their password.
    return GetSavedPasswordUserToken(UserName);
  }
  BYTE *V = 0;
  DWORD nV = 0;
  DWORD Vtype = 0;
  if (!GetRegAnyAlloc(
          HKLM, CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\%08X", UserID),
          L"V", &Vtype, &V, &nV))
    return 0;
  if (!SetRegAny(HKLM, CResStr(SVCKEY L"\\Restore"), CResStr(L"%08X", UserID),
                 Vtype, V, nV, TRUE)) {
    DBGTrace1("Fatal Error, could not create backup of user(%s) credentials!",
              UserName);
    return 0;
  }
  RPC_WSTR p = 0;
  {
    UUID id;
    UuidCreate(&id);
    UuidToString(&id, &p);
  }
  USER_INFO_1003 ui1003 = {0};
  ui1003.usri1003_password = (LPTSTR)p;
  DWORD dwerr = 0;
  NET_API_STATUS st = NetUserSetInfo(NULL, u, 1003, (BYTE *)&ui1003, &dwerr);
  if (st == NERR_UserNotFound)
    return RpcStringFree(&p), GetSavedPasswordUserToken(UserName);
  if (st != NERR_Success) {
    DBGTrace4("NetUserSetInfo(%s,%s,%d) returned %s", (LPCTSTR)u, (LPCTSTR)p,
              dwerr, GetErrorNameStatic(st));
  } else {
    hUser = LogonAsAdmin(UserName, (LPWSTR)p);
  }
  RpcStringFree(&p);
  if (!SetRegAny(HKLM,
                 CResStr(L"SAM\\SAM\\Domains\\Account\\Users\\%08X", UserID),
                 L"V", Vtype, V, nV, TRUE))
    DBGTrace1("Fatal Error, could not restore user(%s) credentials!", UserName);
  free(V);
  if (!RegDelVal(HKLM, CResStr(SVCKEY L"\\Restore"), CResStr(L"%08X", UserID),
                 TRUE))
    DBGTrace1("Fatal Error, could not delete backup of user(%s) credentials!",
              UserName);
  if (hUser)
    g_Tokens.Add(UserName, hUser);
  RestoreUserPasswords();
  return hUser;
}

//////////////////////////////////////////////////////////////////////////////
//
// The code below is heavily based on "GUI-Based RunAsEx" by Zhefu Zhang
//  http://www.codeproject.com/KB/system/RunUser.aspx
//
//////////////////////////////////////////////////////////////////////////////

typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  PVOID RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef UINT(WINAPI *ZwCrTok)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
                              TOKEN_TYPE, PLUID, PLARGE_INTEGER, PTOKEN_USER,
                              PTOKEN_GROUPS, PTOKEN_PRIVILEGES, PTOKEN_OWNER,
                              PTOKEN_PRIMARY_GROUP, PTOKEN_DEFAULT_DACL,
                              PTOKEN_SOURCE);

ZwCrTok ZwCreateToken = NULL;

LPVOID GetFromToken(HANDLE hToken, TOKEN_INFORMATION_CLASS tic) {
  DWORD dw;
  BOOL bRet = FALSE;
  LPVOID lpData = NULL;
  try {
    bRet = GetTokenInformation(hToken, tic, 0, 0, &dw);
    if ((bRet == FALSE) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
      DBGTrace1("GetTokenInformation failed %s", GetLastErrorNameStatic());
      return NULL;
    }
    lpData = (LPVOID)malloc(dw);
    if (lpData) {
      bRet = GetTokenInformation(hToken, tic, lpData, dw, &dw);
      if (!bRet)
        DBGTrace1("GetTokenInformation failed %s", GetLastErrorNameStatic());
    } else
      DBGTrace("malloc failed");
  } catch(...) {} {
    if (!bRet) {
      if (lpData)
        free(lpData);
      lpData = NULL;
    }
  }
  return lpData;
}

PTOKEN_GROUPS AddTokenGroups(PTOKEN_GROUPS pSrcTG, PSID_AND_ATTRIBUTES pAdd) {
  if (!pSrcTG)
    return NULL;
  PTOKEN_GROUPS pDstTG = NULL;
  SIZE_T dwTGlen =
      sizeof(DWORD) + sizeof(SID_AND_ATTRIBUTES) * (pSrcTG->GroupCount + 1);
  DWORD i;
  for (i = 0; i < pSrcTG->GroupCount; i++)
    dwTGlen += GetLengthSid(pSrcTG->Groups[i].Sid);
  dwTGlen += GetLengthSid(pAdd->Sid);
  pDstTG = (PTOKEN_GROUPS)malloc(dwTGlen);
  if (!pDstTG) {
    DBGTrace("malloc failed");
    return 0;
  }
  pDstTG->GroupCount = pSrcTG->GroupCount + 1;
  // SID_AND_ATTRIBUTES* pSA=(SID_AND_ATTRIBUTES*)pDstTG->Groups;
  LPBYTE pBase = (LPBYTE)pDstTG + sizeof(DWORD) +
                 sizeof(SID_AND_ATTRIBUTES) * pDstTG->GroupCount;
  for (i = 0; i < pSrcTG->GroupCount; i++) {
    pDstTG->Groups[i].Attributes = pSrcTG->Groups[i].Attributes;
    memmove(pBase, pSrcTG->Groups[i].Sid, GetLengthSid(pSrcTG->Groups[i].Sid));
    pDstTG->Groups[i].Sid = (PSID)pBase;
    pBase += GetLengthSid(pSrcTG->Groups[i].Sid);
  }
  pDstTG->Groups[i].Attributes = pAdd->Attributes;
  memmove(pBase, pAdd->Sid, GetLengthSid(pAdd->Sid));
  pDstTG->Groups[i].Sid = (PSID)pBase;
  return pDstTG;
}

PTOKEN_PRIMARY_GROUP CreateTokenPrimaryGroup(PSID pSID) {
  if (!pSID)
    return NULL;
  SIZE_T dwTGlen = sizeof(PSID) + GetLengthSid(pSID);
  PTOKEN_PRIMARY_GROUP pTPG = (PTOKEN_PRIMARY_GROUP)malloc(dwTGlen);
  if (!pTPG) {
    DBGTrace("malloc failed");
    return 0;
  }
  LPBYTE pBase = (LPBYTE)pTPG + sizeof(PSID);
  memmove(pBase, pSID, GetLengthSid(pSID));
  pTPG->PrimaryGroup = (PSID)pBase;
  return pTPG;
}

PTOKEN_OWNER CreateTokenOwner(PSID pSID) {
  if (!pSID)
    return NULL;
  SIZE_T dwTGlen = sizeof(PSID) + GetLengthSid(pSID);
  PTOKEN_OWNER pTO = (PTOKEN_OWNER)malloc(dwTGlen);
  if (!pTO) {
    DBGTrace("malloc failed");
    return 0;
  }
  LPBYTE pBase = (LPBYTE)pTO + sizeof(PSID);
  memmove(pBase, pSID, GetLengthSid(pSID));
  pTO->Owner = (PSID)pBase;
  return pTO;
}

PTOKEN_DEFAULT_DACL CreateTokenDefDacl(PACL pACL) {
  if (!pACL)
    return NULL;
  SIZE_T dwlen = sizeof(PACL) + LocalSize(pACL);
  PTOKEN_DEFAULT_DACL pTDACL = (PTOKEN_DEFAULT_DACL)malloc(dwlen);
  if (!pTDACL) {
    DBGTrace("malloc failed");
    return 0;
  }
  LPBYTE pBase = (LPBYTE)pTDACL + sizeof(PACL);
  memmove(pBase, pACL, LocalSize(pACL));
  pTDACL->DefaultDacl = (PACL)pBase;
  return pTDACL;
}

PTOKEN_PRIVILEGES AddPrivileges(PTOKEN_PRIVILEGES pPriv, LPWSTR pAdd) {
  DWORD nPrivs = pPriv->PrivilegeCount;
  LPWSTR s = pAdd;
  while (*s) {
    LUID luid;
    if (LookupPrivilegeValue(0, s, &luid)) {
      bool bAdd = TRUE;
      for (DWORD p = 0; p < pPriv->PrivilegeCount; p++)
        if (memcmp(&pPriv->Privileges[p].Luid, &luid, sizeof(luid)) == 0) {
          bAdd = false;
          break;
        }
      if (bAdd)
        nPrivs++;
    }
    s += wcslen(s) + 1;
  }
  DWORD nBytes = sizeof(DWORD) + nPrivs * sizeof(LUID_AND_ATTRIBUTES);
  PTOKEN_PRIVILEGES ptp = (PTOKEN_PRIVILEGES)calloc(nBytes, 1);
  if (!ptp) {
    DBGTrace("calloc failed");
    return 0;
  }
  memmove(ptp, pPriv,
          sizeof(DWORD) + pPriv->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES));
  s = pAdd;
  nPrivs = pPriv->PrivilegeCount;
  while (*s) {
    LUID luid;
    if (LookupPrivilegeValue(0, s, &luid)) {
      bool bAdd = TRUE;
      for (DWORD p = 0; p < pPriv->PrivilegeCount; p++)
        if (memcmp(&pPriv->Privileges[p].Luid, &luid, sizeof(luid)) == 0) {
          bAdd = false;
          break;
        }
      if (bAdd) {
        memmove(&ptp->Privileges[nPrivs].Luid, &luid, sizeof(luid));
        nPrivs++;
      }
    }
    s += wcslen(s) + 1;
  }
  ptp->PrivilegeCount = nPrivs;
  for (DWORD p = 0; p < ptp->PrivilegeCount; p++) {
    TCHAR s[MAX_PATH] = {0};
    DWORD l = MAX_PATH;
    if (LookupPrivilegeName(0, &ptp->Privileges[p].Luid, s, &l)) {
      if (_tcsicmp(s, SE_CHANGE_NOTIFY_NAME) == 0)
        ptp->Privileges[p].Attributes =
            SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED;
      else if (_tcsicmp(s, SE_CREATE_GLOBAL_NAME) == 0)
        ptp->Privileges[p].Attributes =
            SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED;
      else if (_tcsicmp(s, SE_LOAD_DRIVER_NAME) == 0)
        ptp->Privileges[p].Attributes = SE_PRIVILEGE_ENABLED;
      else if (_tcsicmp(s, SE_IMPERSONATE_NAME) == 0)
        ptp->Privileges[p].Attributes =
            SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED;
      else if (_tcsicmp(s, SE_UNDOCK_NAME) == 0)
        ptp->Privileges[p].Attributes = SE_PRIVILEGE_ENABLED;
      else if (_tcsicmp(s, SE_DEBUG_NAME) == 0)
        ptp->Privileges[p].Attributes = 0;
    }
  }
  return ptp;
}

TOKEN_STATISTICS g_AdminTStat = {0};

HANDLE GetAdminToken(DWORD SessionID) {
  if ((g_AdminTStat.AuthenticationId.HighPart |
       g_AdminTStat.AuthenticationId.LowPart) == 0) {
    DBGTrace("GetAdminToken failed because g_AdminTStat is not set!");
    return FALSE;
  }
  if (!EnablePrivilege(SE_CREATE_TOKEN_NAME)) {
    DBGTrace("EnablePrivilege(SE_CREATE_TOKEN_NAME) failed");
    // return FALSE;
  }
  HANDLE hUser = NULL;
  HANDLE hShell = NULL;
  PSID LogonSID = NULL;
  PSID UserSID = NULL;
  PSID AdminSID = NULL;
  PTOKEN_GROUPS ptg = NULL;
  PTOKEN_PRIVILEGES lpPrivToken = NULL;
  PTOKEN_PRIMARY_GROUP lpPriGrp = NULL;
  PTOKEN_DEFAULT_DACL lpDaclToken = NULL;
  PTOKEN_OWNER pTO = NULL;
  try {
    HANDLE hShell = GetSessionUserToken(SessionID);
    if (!hShell) {
      DBGTrace("GetSessionUserToken failed");
       throw 123;
    }
    // Is the Shell token a Vista Split token?
    if (GetElevatedToken(hShell)) {
      SetHighIL(hShell);
      hUser = hShell;
      hShell = 0;
       throw 123;
    }
    // Copy Logon SID from the Shell Process of SessionID:
    LogonSID = GetLogonSid(hShell);
    if (!LogonSID) {
      DBGTrace("GetLogonSid failed");
       throw 123;
    }
    // Copy TokenSource from the Shell Process of SessionID:
    TOKEN_SOURCE tsrc = {0};
    DWORD n;
    CHK_BOOL_FN(
        GetTokenInformation(hShell, TokenSource, &tsrc, sizeof(tsrc), &n));
    strcpy(tsrc.SourceName, "SuRun");
    // Initialize TOKEN_GROUPS
    ptg = (PTOKEN_GROUPS)(GetFromToken(hShell, TokenGroups));
    if (ptg == NULL) {
      DBGTrace("GetFromToken failed");
       throw 123;
    }
    for (DWORD i = 0; i < ptg->GroupCount; i++)
      ptg->Groups[i].Attributes &= ~SE_GROUP_OWNER;
    // Initialize Admin SID
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&sidAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &AdminSID)) {
      DBGTrace1("AllocateAndInitializeSid failed %s", GetLastErrorNameStatic());
       throw 123;
    }
    // copy Admin Sid to token groups
    SID_AND_ATTRIBUTES sia;
    sia.Sid = AdminSID;
    sia.Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                     SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_OWNER;
    PTOKEN_GROUPS p = AddTokenGroups(ptg, &sia);
    if (!p) {
      DBGTrace("AddTokenGroups failed");
       throw 123;
    }
    free(ptg);
    ptg = p;
    // Set Admin SID as the Tokens Primary Group
    lpPriGrp = CreateTokenPrimaryGroup(AdminSID);
    if (!lpPriGrp)
      DBGTrace("CreateTokenPrimaryGroup failed");
    // Set Token User
    UserSID = GetTokenUserSID(hShell);
    if (!UserSID)
      DBGTrace("GetTokenUserSID failed");
    TOKEN_USER userToken = {{UserSID, 0}};
    // Set Admin SID as the Token Owner
    pTO = CreateTokenOwner(GetOwnerAdminGrp ? AdminSID : UserSID);
    if (!pTO)
      DBGTrace("CreateTokenOwner failed");
    // Privileges
    lpPrivToken = (PTOKEN_PRIVILEGES)(GetFromToken(hShell, TokenPrivileges));
    if (!lpPrivToken)
      DBGTrace("GetFromToken failed");
    // merge Privileges of User and Admin
    WCHAR un[MAX_PATH + MAX_PATH + 1];
    if (!GetSIDUserName(AdminSID, un))
      DBGTrace("GetSIDUserName failed");
    LPWSTR aRights = GetAccountPrivileges(un);
    if (!aRights)
      DBGTrace("GetAccountPrivileges failed");
    PTOKEN_PRIVILEGES priv = AddPrivileges(lpPrivToken, aRights);
    free(aRights);
    if (!priv) {
      DBGTrace("AddPrivileges failed");
       throw 123;
    }
    free(lpPrivToken);
    lpPrivToken = priv;
    // Set Default DACL, Deny user Access but Allow Admins access, so User
    // running as admin can access this token but user running as user cannot
    lpDaclToken = (PTOKEN_DEFAULT_DACL)(GetFromToken(hShell, TokenDefaultDacl));
    if (!lpDaclToken)
      DBGTrace("GetFromToken failed");
    PACL DefDACL = SetAdminDenyUserAccess(lpDaclToken->DefaultDacl, UserSID);
    if (DefDACL) {
      free(lpDaclToken);
      lpDaclToken = CreateTokenDefDacl(DefDACL);
      LocalFree(DefDACL);
    }
    //
    OBJECT_ATTRIBUTES oa = {sizeof(oa), 0, 0, 0, 0, 0};
    // Create the token
    if (!ZwCreateToken)
      ZwCreateToken = (ZwCrTok)GetProcAddress(GetModuleHandleA("ntdll.dll"),
                                              "ZwCreateToken");
    if (!ZwCreateToken) {
      DBGTrace1("GetProcAddress(ZwCreateToken) failed: %s",
                GetLastErrorNameStatic());
       throw 123;
    }
    NTSTATUS ntStatus = ZwCreateToken(
        &hUser, READ_CONTROL | TOKEN_ALL_ACCESS, &oa, TokenPrimary,
        &g_AdminTStat.AuthenticationId, &g_AdminTStat.ExpirationTime,
        &userToken, ptg, lpPrivToken, pTO, lpPriGrp, lpDaclToken, &tsrc);
    // 0xc000005a invalid owner
    if (ntStatus != STATUS_SUCCESS)
      DBGTrace1("GetAdminToken ZwCreateToken Failed: 0x%08X", ntStatus);
    SetHighIL(hUser);
  } catch(...) {} {
    if (hShell)
      CloseHandle(hShell);
    if (LogonSID)
      free(LogonSID);
    if (UserSID)
      free(UserSID);
    if (AdminSID)
      FreeSid(AdminSID);
    if (ptg)
      free(ptg);
    if (lpPrivToken)
      free(lpPrivToken);
    if (lpPriGrp)
      free(lpPriGrp);
    if (pTO)
      free(pTO);
    if (lpDaclToken)
      free(lpDaclToken);
  }
  return hUser;
}
