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

/////////////////////////////////////////////////////////////////////////////
//
// Most of this is heavily based on SuDown (http://sudown.sourceforge.net)
//
/////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0A00
#define WINVER 0x0A00
#include <windows.h>
#include <TCHAR.h>
#include <lm.h>
#include <malloc.h>
#include <shlwapi.h>

#include "UserGroups.h"
#include "DBGTrace.h"
#include "Helpers.h"
#include "LSALogon.h"
#include "LogonDlg.h"
#include "ResStr.h"
#include "Resource.h"
#include "Setup.h"


#ifdef DoDBGTrace
#include <Sddl.h>
#pragma comment(lib, "Advapi32.lib")
#endif //DoDBGTrace

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Netapi32.lib")

/////////////////////////////////////////////////////////////////////////////
//
// CreateSuRunnersGroup
//
/////////////////////////////////////////////////////////////////////////////
void CreateSuRunnersGroup() {
  if (GetUseSuRunGrp) {
    LOCALGROUP_INFO_1 lgri1 = {SURUNNERSGROUP, CResStr(IDS_GRPDESC)};
    DWORD error;
    NetLocalGroupAdd(NULL, 1, (LPBYTE)&lgri1, &error);
  }
}

/////////////////////////////////////////////////////////////////////////////
//
// DeleteSuRunnersGroup
//
/////////////////////////////////////////////////////////////////////////////
BOOL DeleteTempAdmin() // SuRun 1.2.0.7 beta 13 used an own account, we need to
                       // delete that
{
  TCHAR u[UNLEN] = {0};
  GetRegStr(HKLM, SURUNKEY, L"SuRunHelpUser", u, UNLEN);
  if (u[0]) {
    RegDelVal(HKLM, SURUNKEY, L"SuRunHelpUser");
    NET_API_STATUS st = NetUserDel(NULL, u);
    if (st != NERR_Success)
      DBGTrace2("NetUserDel(%s) returned %s", (LPCTSTR)u,
                GetErrorNameStatic(st));
    return st == NERR_Success;
  }
  return TRUE;
}

void DeleteSuRunnersGroup() {
  NetLocalGroupDel(NULL, SURUNNERSGROUP);
  DeleteTempAdmin();
}

/////////////////////////////////////////////////////////////////////////////
//
// GetGroupName
//
/////////////////////////////////////////////////////////////////////////////
BOOL GetGroupName(DWORD Rid, LPWSTR Name, PDWORD cchName) {
  SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
  PSID pSid;
  BOOL bSuccess = FALSE;
  if (AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID, Rid, 0, 0,
                               0, 0, 0, 0, &pSid)) {
    WCHAR DomainName[UNLEN + 1];
    DWORD DnLen = UNLEN;
    SID_NAME_USE snu;
    bSuccess =
        LookupAccountSidW(0, pSid, Name, cchName, DomainName, &DnLen, &snu);
    FreeSid(pSid);
  }
  return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
//
// AlterGroupMembership
//
//  Adds or removes "DOMAIN\User" from the local group "Group"
/////////////////////////////////////////////////////////////////////////////
DWORD AlterGroupMember(LPCWSTR Group, LPCWSTR DomainAndName, BOOL bAdd) {
  LOCALGROUP_MEMBERS_INFO_3 lgrmi3 = {0};
  lgrmi3.lgrmi3_domainandname = (LPWSTR)DomainAndName;
  if (bAdd)
    return NetLocalGroupAddMembers(NULL, Group, 3, (LPBYTE)&lgrmi3, 1);
  return NetLocalGroupDelMembers(NULL, Group, 3, (LPBYTE)&lgrmi3, 1);
}

DWORD AlterGroupMember(DWORD Rid, LPCWSTR DomainAndName, BOOL bAdd) {
  DWORD cchG = GNLEN;
  WCHAR Group[GNLEN + 1] = {0};
  if (!GetGroupName(Rid, Group, &cchG))
    return NERR_GroupNotFound;
  return AlterGroupMember(Group, DomainAndName, bAdd);
}

/////////////////////////////////////////////////////////////////////////////
//
// IsInGroup
//
//  Checks if "DOMAIN\User" is a member of the group
/////////////////////////////////////////////////////////////////////////////
BOOL IsInGroupDirect(LPCWSTR Group, LPCWSTR DomainAndName) {
  if ((DomainAndName == 0) || (*DomainAndName == 0))
    return false;
  // CTimeLog l(L"IsInGroupDirect(%s,%s)",Group,DomainAndName);
  DWORD result = 0;
  NET_API_STATUS status;
  LPLOCALGROUP_MEMBERS_INFO_3 Members = NULL;
  DWORD num = 0;
  DWORD total = 0;
  DWORD_PTR resume = 0;
  DWORD i;
  do {
    status =
        NetLocalGroupGetMembers(NULL, Group, 3, (LPBYTE *)&Members,
                                MAX_PREFERRED_LENGTH, &num, &total, &resume);
    if (((status == NERR_Success) || (status == ERROR_MORE_DATA)) &&
        (result == 0)) {
      if (Members)
        for (i = 0; (i < total); i++) {
          if (wcscmp(Members[i].lgrmi3_domainandname, DomainAndName) == 0) {
            NetApiBufferFree(Members);
            return TRUE;
          }
        }
    } else {
      DBGTrace3("IsInGroupDirect(%s,%s): NetLocalGroupGetMembers failed: %s",
                Group, DomainAndName, GetErrorNameStatic(status));
    }
  } while (status == ERROR_MORE_DATA);
  NetApiBufferFree(Members);
  return FALSE;
}

BOOL IsInGroup(LPCWSTR Group, LPCWSTR DomainAndName, DWORD SessionId) {
  // CTimeLog l(L"IsInGroup(%s,%s)",Group,DomainAndName);
  // try to find user in local group
  if ((!DomainAndName) || (*DomainAndName == 0))
    return FALSE;
  NET_API_STATUS status;
  LPLOCALGROUP_USERS_INFO_0 Users = 0;
  DWORD num = 0;
  DWORD total = 0;
  CImpersonateSessionUser ilu(SessionId);
  status = NetUserGetLocalGroups(NULL, DomainAndName, 0, LG_INCLUDE_INDIRECT,
                                 (LPBYTE *)&Users, MAX_PREFERRED_LENGTH, &num,
                                 &total);
  if ((((status == NERR_Success) || (status == ERROR_MORE_DATA))) && Users) {
    for (DWORD i = 0; (i < total); i++) {
      if (wcsicmp(Users[i].lgrui0_name, Group) == 0) {
        NetApiBufferFree(Users);
        return TRUE;
      }
    }
    NetApiBufferFree(Users);
    Users = 0;
  } else {
    DBGTrace3("IsInGroup(%s,%s): NetUserGetLocalGroups failed: %s", Group,
              DomainAndName, GetErrorNameStatic(status));
    return IsInGroupDirect(Group, DomainAndName);
  }
  return FALSE;
}

BOOL IsInGroup(DWORD Rid, LPCWSTR DomainAndName, DWORD SessionId) {
  DWORD cchG = GNLEN;
  WCHAR Group[GNLEN + 1] = {0};
  if (!GetGroupName(Rid, Group, &cchG))
    return FALSE;
  return IsInGroup(Group, DomainAndName, SessionId);
}

/////////////////////////////////////////////////////////////////////////////
//
// IsInSuRunners
//
//  Checks if "DOMAIN\User" is a member of the SuRunners localgroup
/////////////////////////////////////////////////////////////////////////////
BOOL IsInSuRunners(LPCWSTR DomainAndName, DWORD SessionId) {
  if ((DomainAndName == 0) || (*DomainAndName == 0))
    return false;
  if (!GetUseSuRunGrp)
    return TRUE;
  if (IsInGroup(SURUNNERSGROUP, DomainAndName, SessionId))
    return TRUE;
  // DBGTrace1("IsInSuRunners(%s): Not in SuRunners!
  // ->DelUsrSettings",DomainAndName);
  DelUsrSettings(DomainAndName);
  return FALSE;
}

DWORD IsInSuRunnersOrAdmins(LPCWSTR DomainAndName, DWORD SessionID) {
  // CTimeLog l(L"IsInSuRunnersOrAdmins(%s)",DomainAndName);
  if ((DomainAndName == 0) || (*DomainAndName == 0))
    return 0;
  DWORD dwRet = 0;
  {
    CImpersonateSessionUser ilu(SessionID);
    DWORD cchAG = GNLEN;
    WCHAR AGroup[GNLEN + 1] = {0};
    GetGroupName(DOMAIN_ALIAS_RID_ADMINS, AGroup, &cchAG);
    if (!GetUseSuRunGrp)
      dwRet |= IS_IN_SURUNNERS;
    // try to find user in local group
    NET_API_STATUS status;
    LPLOCALGROUP_USERS_INFO_0 Users = 0;
    DWORD num = 0;
    DWORD total = 0;
    status = NetUserGetLocalGroups(NULL, DomainAndName, 0, LG_INCLUDE_INDIRECT,
                                   (LPBYTE *)&Users, MAX_PREFERRED_LENGTH, &num,
                                   &total);
    if ((((status == NERR_Success) || (status == ERROR_MORE_DATA))) && Users) {
      for (DWORD i = 0; (i < total); i++) {
        if (wcsicmp(Users[i].lgrui0_name, AGroup) == 0) {
          dwRet |= IS_IN_ADMINS;
          if (UACEnabled)
            dwRet |= IS_SPLIT_ADMIN;
        } else if (wcsicmp(Users[i].lgrui0_name, SURUNNERSGROUP) == 0)
          dwRet |= IS_IN_SURUNNERS;
      }
      NetApiBufferFree(Users);
      Users = 0;
    } else {
      DBGTrace2("IsInSuRunnersOrAdmins(%s): NetUserGetLocalGroups failed: %s",
                DomainAndName, GetErrorNameStatic(status));
      if (IsInGroupDirect(AGroup, DomainAndName))
        dwRet |= IS_IN_ADMINS;
      if (IsInGroupDirect(SURUNNERSGROUP, DomainAndName))
        dwRet |= IS_IN_SURUNNERS;
    }
  }
  if ((dwRet & IS_IN_SURUNNERS) == 0) {
    // DBGTrace1("IsInSuRunnersOrAdmins(%s): Not in SuRunners!
    // ->DelUsrSettings",DomainAndName);
    DelUsrSettings(DomainAndName);
  }
  return dwRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  BecomeSuRunner
//
//////////////////////////////////////////////////////////////////////////////
BOOL BecomeSuRunner(LPCTSTR UserName, DWORD SessionID, bool bIsInAdmins,
                    bool bIsSplitAdmin, BOOL bHimSelf, HWND hwnd) {
  if ((UserName == 0) || (*UserName == 0))
    return 0;
  // Is User member of SuRunners?
  CResStr sCaption(IDS_APPNAME);
  if (bHimSelf) {
    _tcscat(sCaption, L" (");
    _tcscat(sCaption, UserName);
    _tcscat(sCaption, L")");
  }
  // Is User member of Administrators?
  if (bIsInAdmins && (!bIsSplitAdmin)) {
    // Whoops...need to become a User!
    if (SafeMsgBox(hwnd,
                   CBigResStr((bHimSelf ? IDS_ASKSURUNNER : IDS_ASKSURUNNER1),
                              UserName, UserName),
                   sCaption,
                   MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO)
      return FALSE;
    DWORD dwRet = AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, UserName, 0);
    if (dwRet) {
      // Built in (or Domain-)Admin... Bail out!
      SafeMsgBox(hwnd,
                 bHimSelf
                     ? CBigResStr(IDS_BUILTINADMIN, GetErrorNameStatic(dwRet))
                     : CBigResStr(IDS_BUILTINADMIN1, UserName,
                                  GetErrorNameStatic(dwRet)),
                 sCaption, MB_ICONSTOP);
      return FALSE;
    }
    dwRet = AlterGroupMember(DOMAIN_ALIAS_RID_USERS, UserName, 1);
    if (dwRet && (dwRet != ERROR_MEMBER_IN_ALIAS)) {
      AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, UserName, 1);
      SafeMsgBox(hwnd,
                 bHimSelf
                     ? CBigResStr(IDS_NOADD2USERS, GetErrorNameStatic(dwRet))
                     : CBigResStr(IDS_NOADD2USERS1, UserName,
                                  GetErrorNameStatic(dwRet)),
                 sCaption, MB_ICONSTOP);
      return FALSE;
    }
    dwRet = (AlterGroupMember(SURUNNERSGROUP, UserName, 1) != 0);
    if (dwRet && (dwRet != ERROR_MEMBER_IN_ALIAS)) {
      SafeMsgBox(hwnd,
                 bHimSelf
                     ? CBigResStr(IDS_SURUNNER_ERR, GetErrorNameStatic(dwRet))
                     : CBigResStr(IDS_SURUNNER_ERR1, UserName,
                                  GetErrorNameStatic(dwRet)),
                 sCaption, MB_ICONSTOP);
      return FALSE;
    }
    if (bHimSelf)
      SafeMsgBox(hwnd, CBigResStr(IDS_LOGOFFON), sCaption, MB_ICONINFORMATION);
    return TRUE;
  }
  // Vista, leave user in Admins to not affect UAC
  if (bIsSplitAdmin &&
      (SafeMsgBox(hwnd,
                  CBigResStr((bHimSelf ? IDS_ASKSURUNNER2 : IDS_ASKSURUNNER3),
                             UserName),
                  sCaption,
                  MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO))
    return FALSE;
  if ((!bIsSplitAdmin) && bHimSelf && (!LogonAdmin(SessionID, IDS_NOSURUNNER)))
    return FALSE;
  DWORD dwRet = (AlterGroupMember(SURUNNERSGROUP, UserName, 1) != 0);
  if (dwRet && (dwRet != ERROR_MEMBER_IN_ALIAS)) {
    SafeMsgBox(hwnd,
               bHimSelf
                   ? CBigResStr(IDS_SURUNNER_ERR, GetErrorNameStatic(dwRet))
                   : CBigResStr(IDS_SURUNNER_ERR1, UserName,
                                GetErrorNameStatic(dwRet)),
               sCaption, MB_ICONSTOP);
    return FALSE;
  }
  if (bHimSelf)
    SafeMsgBox(hwnd, CBigResStr(IDS_SURUNNER_OK), sCaption, MB_ICONINFORMATION);
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
// User list
//
/////////////////////////////////////////////////////////////////////////////

// Well known user groups for filling the list in LogonDlg
const DWORD UserGroups[] = {DOMAIN_ALIAS_RID_ADMINS,
                            DOMAIN_ALIAS_RID_POWER_USERS,
                            DOMAIN_ALIAS_RID_USERS, DOMAIN_ALIAS_RID_GUESTS};

USERLIST::USERLIST() {
  nUsers = 0;
  User = 0;
  m_bSkipAdmins = FALSE;
}

USERLIST::~USERLIST() {
  DeleteUserBitmaps();
  free(User);
}

LPTSTR USERLIST::GetUserName(int nUser) {
  if ((nUser < 0) || (nUser >= nUsers))
    return 0;
  return User[nUser].UserName;
}

HBITMAP USERLIST::GetUserBitmap(int nUser) {
  if ((nUser < 0) || (nUser >= nUsers))
    return 0;
  return User[nUser].UserBitmap;
}

HBITMAP USERLIST::GetUserBitmap(LPTSTR UserName) {
  if ((UserName == 0) || (*UserName == 0))
    return 0;
  TCHAR un[2 * UNLEN + 2];
  _tcscpy(un, UserName);
  PathStripPath(un);
  for (int i = 0; i < nUsers; i++) {
    TCHAR UN[2 * UNLEN + 2] = {0};
    _tcscpy(UN, User[i].UserName);
    PathStripPath(UN);
    if (_tcsicmp(un, UN) == 0)
      return User[i].UserBitmap;
  }
  return 0;
}

SIZE USERLIST::GetUserBitmapSize(int nUser) {
  SIZE s = {0};
  if ((nUser < 0) || (nUser >= nUsers))
    return s;
  return User[nUser].UserBitmapSize;
}

SIZE USERLIST::GetUserBitmapSize(LPTSTR UserName) {
  SIZE s = {0};
  if ((UserName == 0) || (*UserName == 0))
    return s;
  TCHAR un[2 * UNLEN + 2];
  _tcscpy(un, UserName);
  PathStripPath(un);
  for (int i = 0; i < nUsers; i++) {
    TCHAR UN[2 * UNLEN + 2] = {0};
    _tcscpy(UN, User[i].UserName);
    PathStripPath(UN);
    if (_tcsicmp(un, UN) == 0)
      return User[i].UserBitmapSize;
  }
  return s;
}

void USERLIST::DeleteUserBitmap(int i) {
  if (User[i].UserBitmap) {
    DeleteObject(User[i].UserBitmap);
    User[i].UserBitmap = 0;
    User[i].UserBitmapSize.cx = 0;
    User[i].UserBitmapSize.cy = 0;
  }
}

void USERLIST::DeleteUserBitmaps() {
  for (int i = 0; i < nUsers; i++)
    DeleteUserBitmap(i);
}

void USERLIST::LoadUserBitmap(int i) {
  if (User[i].UserBitmap)
    return;
  User[i].UserBitmap = ::LoadUserBitmap(User[i].UserName);
  if (User[i].UserBitmap) {
    BITMAP b;
    GetObject(User[i].UserBitmap, sizeof(BITMAP), &b);
    User[i].UserBitmapSize.cx = b.bmWidth;
    User[i].UserBitmapSize.cy = b.bmHeight;
  }
}

void USERLIST::LoadUserBitmaps() {
  for (int i = 0; i < nUsers; i++)
    LoadUserBitmap(i);
}

int USERLIST::FindUser(LPTSTR UserName) {
  if ((UserName == 0) || (*UserName == 0))
    return -1;
  for (int i = 0; i < nUsers; i++)
    if (_tcsicmp(User[i].UserName, UserName) == 0)
      return i;
  return -1;
}

void USERLIST::SetUsualUsers(DWORD SessionId, BOOL bScanDomain) {
  DeleteUserBitmaps();
  free(User);
  User = 0;
  nUsers = 0;
  {
    CImpersonateSessionUser ilu(SessionId);
    for (int g = 0; g < countof(UserGroups); g++)
      AddGroupUsers(UserGroups[g], bScanDomain);
  }
  LoadUserBitmaps();
}

void USERLIST::SetGroupUsers(LPWSTR GroupName, DWORD SessionId,
                             BOOL bScanDomain) {
  DeleteUserBitmaps();
  free(User);
  User = 0;
  nUsers = 0;
  {
    CImpersonateSessionUser ilu(SessionId);
    if (_tcscmp(GroupName, _T("*")) == 0)
      AddAllUsers(bScanDomain);
    else
      AddGroupUsers(GroupName, bScanDomain);
  }
  LoadUserBitmaps();
}

void USERLIST::SetGroupUsers(DWORD WellKnownGroup, DWORD SessionId,
                             BOOL bScanDomain) {
  DWORD GNLen = GNLEN;
  WCHAR GroupName[GNLEN + 1];
  if (GetGroupName(WellKnownGroup, GroupName, &GNLen))
    SetGroupUsers(GroupName, SessionId, bScanDomain);
}

void USERLIST::SetSurunnersUsers(LPCTSTR CurUser, DWORD SessionId,
                                 BOOL bScanDomain) {
  if (GetUseSuRunGrp)
    SetGroupUsers(SURUNNERSGROUP, SessionId, bScanDomain);
  else {
    // Add all users
    m_bSkipAdmins = TRUE;
    SetGroupUsers(_T("*"), SessionId, bScanDomain);
    if (CurUser && *CurUser)
      Add(CurUser);
    m_bSkipAdmins = FALSE;
  }
}

static USERDATA *UsrRealloc(USERDATA *User, int nUsers) {
  void *p = User;
  if (p == 0)
    p = malloc(nUsers * sizeof(USERDATA) + 16384);
  else if (_msize(p) < nUsers * sizeof(USERDATA))
    p = realloc(User, nUsers * sizeof(USERDATA) + 16384);
  if (!p)
    DBGTrace2("realloc(%x,%d) failed!", User, nUsers * sizeof(USERDATA));
  return (USERDATA *)p;
}

void USERLIST::Add(LPCWSTR UserName) {
  if ((UserName == 0) || (*UserName == 0))
    return;
  if (m_bSkipAdmins && IsInGroup(DOMAIN_ALIAS_RID_ADMINS, UserName, (DWORD)-1))
    return;
  int j = 0;
  for (; j < nUsers; j++) {
    int cr = _tcsicmp(User[j].UserName, UserName);
    if (cr == 0)
      return;
    if (cr > 0) {
      if (j < nUsers) {
        User = UsrRealloc(User, nUsers + 1);
        if (!User)
          return;
        memmove(&User[j + 1], &User[j], (nUsers - j) * sizeof(User[0]));
      }
      break;
    }
  }
  if (j >= nUsers)
    User = UsrRealloc(User, nUsers + 1);
  //  DBGTrace1("-->AddUser: %s",UserName);
  zero(User[j]);
  _tcscpy(User[j].UserName, UserName);
  nUsers++;
  return;
}

static void MsgLoop() {
  MSG msg;
  int count = 0;
  while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && (count++ < 100)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void USERLIST::AddGroupUsers(LPWSTR GroupName, BOOL bScanDomain) {
  if ((GroupName == 0) || (*GroupName == 0))
    return;
  //  DBGTrace1("AddGroupUsers for Group %s",GroupName);
  TCHAR cn[2 * UNLEN + 2] = {0};
  DWORD cnl = UNLEN;
  GetComputerName(cn, &cnl);
  // First try to add network group members
  if (bScanDomain) {
    TCHAR dn[2 * UNLEN + 2] = {0};
    _tcscpy(dn, GroupName);
    PathRemoveFileSpec(dn);
    LPTSTR dc = 0;
    if (dn[0] /*only domain groups!*/ && (_tcsicmp(dn, cn) != 0) &&
        (NetGetAnyDCName(0, dn, (BYTE **)&dc) == NERR_Success)) {

      TCHAR gn[2 * UNLEN + 2] = {0};
      _tcscpy(gn, GroupName);
      PathStripPath(gn);
      DWORD_PTR i = 0;
      DWORD res = ERROR_MORE_DATA;
      for (; res == ERROR_MORE_DATA;) {
        LPBYTE pBuff = 0;
        DWORD dwRec = 0;
        DWORD dwTot = 0;
        MsgLoop();
        res = NetGroupGetUsers(dc, gn, 0, &pBuff, MAX_PREFERRED_LENGTH, &dwRec,
                               &dwTot, &i);
        if ((res != ERROR_SUCCESS) && (res != ERROR_MORE_DATA)) {
          DBGTrace3("NetGroupGetUsers(%s,%s) failed: %s", dc, gn,
                    GetErrorNameStatic(res));
          break;
        }
        for (GROUP_USERS_INFO_0 *p = (GROUP_USERS_INFO_0 *)pBuff; dwRec > 0;
             dwRec--, p++) {
          USER_INFO_2 *b = 0;
          MsgLoop();
          NetUserGetInfo(dc, p->grui0_name, 2, (LPBYTE *)&b);
          if (b) {
            if ((b->usri2_flags & UF_ACCOUNTDISABLE) == 0)
              Add(CResStr(L"%s\\%s", dn, p->grui0_name));
            NetApiBufferFree(b);
          } else
            DBGTrace3("NetUserGetInfo(%s,%s) failed: %s", dc, p->grui0_name,
                      GetErrorNameStatic(res));
        }
        NetApiBufferFree(pBuff);
      }
      NetApiBufferFree(dc);
    }
  }

  // second try to add local group members
  DWORD_PTR i = 0;
  DWORD res = ERROR_MORE_DATA;
  for (; res == ERROR_MORE_DATA;) {
    LPBYTE pBuff = 0;
    DWORD dwRec = 0;
    DWORD dwTot = 0;
    MsgLoop();
    res = NetLocalGroupGetMembers(0, GroupName, 2, &pBuff, MAX_PREFERRED_LENGTH,
                                  &dwRec, &dwTot, &i);
    if ((res != ERROR_SUCCESS) && (res != ERROR_MORE_DATA)) {
      DBGTrace2("NetLocalGroupGetMembers(%s) failed: %s", GroupName,
                GetErrorNameStatic(res));
      break;
    }
    for (LOCALGROUP_MEMBERS_INFO_2 *p = (LOCALGROUP_MEMBERS_INFO_2 *)pBuff;
         dwRec > 0; dwRec--, p++) {
#ifdef DoDBGTrace
//      LPTSTR sSID=0;
//      ConvertSidToStringSid(p->lgrmi2_sid,&sSID);
//      DBGTrace4("Group %s Name: %s; Type: %d; SID: %s",
//        GroupName,p->lgrmi2_domainandname,p->lgrmi2_sidusage,sSID);
//      LocalFree(sSID);
#endif //DoDBGTrace
      switch (p->lgrmi2_sidusage) {
      case SidTypeUser: {
        TCHAR un[2 * UNLEN + 2] = {0};
        TCHAR dn[2 * UNLEN + 2] = {0};
        _tcscpy(un, p->lgrmi2_domainandname);
        PathStripPath(un);
        _tcscpy(dn, p->lgrmi2_domainandname);
        PathRemoveFileSpec(dn);
        USER_INFO_2 *b = 0;
        LPTSTR dc = 0;
        MsgLoop();
        if (dn[0] /*only domain groups!*/ && (_tcsicmp(dn, cn) != 0))
          NetGetAnyDCName(0, dn, (BYTE **)&dc);
        NetUserGetInfo(dc, un, 2, (LPBYTE *)&b);
        if (b) {
          if ((b->usri2_flags & UF_ACCOUNTDISABLE) == 0)
            Add(p->lgrmi2_domainandname);
          NetApiBufferFree(b);
        } else
          // User not found: Domain Controller not present! Add user to list
          Add(p->lgrmi2_domainandname);
        if (dc)
          NetApiBufferFree(dc);
      } break;
      case SidTypeComputer:
      case SidTypeDomain:
      case SidTypeGroup:
      case SidTypeWellKnownGroup:
        // Groups can be members of Groups...
        AddGroupUsers(p->lgrmi2_domainandname, bScanDomain);
        break;
      }
    }
    NetApiBufferFree(pBuff);
  }
}

void USERLIST::AddGroupUsers(DWORD WellKnownGroup, BOOL bScanDomain) {
  DWORD GNLen = GNLEN;
  WCHAR GroupName[GNLEN + 1];
  if (GetGroupName(WellKnownGroup, GroupName, &GNLen))
    AddGroupUsers(GroupName, bScanDomain);
}

void USERLIST::AddAllUsers(BOOL bScanDomain) {
  DWORD_PTR i = 0;
  DWORD res = ERROR_MORE_DATA;
  for (; res == ERROR_MORE_DATA;) {
    LPBYTE pBuff = 0;
    DWORD dwRec = 0;
    DWORD dwTot = 0;
    MsgLoop();
    res = NetLocalGroupEnum(NULL, 0, &pBuff, MAX_PREFERRED_LENGTH, &dwRec,
                            &dwTot, &i);
    if ((res != ERROR_SUCCESS) && (res != ERROR_MORE_DATA)) {
      DBGTrace1("NetLocalGroupEnum failed: %s", GetErrorNameStatic(res));
      return;
    }
    for (LOCALGROUP_INFO_0 *p = (LOCALGROUP_INFO_0 *)pBuff; dwRec > 0;
         dwRec--, p++)
      AddGroupUsers(p->lgrpi0_name, bScanDomain);
    NetApiBufferFree(pBuff);
  }
  if (bScanDomain) {
    i = 0;
    res = ERROR_MORE_DATA;
    for (; res == ERROR_MORE_DATA;) {
      LPBYTE pBuff = 0;
      DWORD dwRec = 0;
      DWORD dwTot = 0;
      MsgLoop();
      res = NetGroupEnum(NULL, 0, &pBuff, MAX_PREFERRED_LENGTH, &dwRec, &dwTot,
                         &i);
      if ((res != ERROR_SUCCESS) && (res != ERROR_MORE_DATA)) {
        DBGTrace1("NetLocalGroupEnum failed: %s", GetErrorNameStatic(res));
        return;
      }
      for (GROUP_INFO_0 *p = (GROUP_INFO_0 *)pBuff; dwRec > 0; dwRec--, p++)
        AddGroupUsers(p->grpi0_name, bScanDomain);
      NetApiBufferFree(pBuff);
    }
  }
}
