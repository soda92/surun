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

#pragma once
#include <LMCONS.H>
#include <windows.h>

// SuRun users group name
#define SURUNNERSGROUP _T("SuRunners")

// this Creates the local user group "SuRunners"
void CreateSuRunnersGroup();

// this Deletes the local user group "SuRunners"
void DeleteSuRunnersGroup();

// Get the name of a well known group
BOOL GetGroupName(DWORD Rid, LPWSTR Name, PDWORD cchName);

// put/remove User "DomainAndName" in/from Group
DWORD AlterGroupMember(LPCWSTR Group, LPCWSTR DomainAndName, BOOL bAdd);
// put/remove User "DomainAndName" in/from Group GetGroupName("Rid")
DWORD AlterGroupMember(DWORD Rid, LPCWSTR DomainAndName, BOOL bAdd);

// is User "DomainAndName" member of Group GetGroupName("Rid")?
BOOL IsInGroup(DWORD Rid, LPCWSTR DomainAndName, DWORD SessionID);

// is User "DomainAndName" member of SuRunners?
BOOL IsInSuRunners(LPCWSTR DomainAndName, DWORD SessionID);
#define IsInAdmins(u, s) IsInGroup(DOMAIN_ALIAS_RID_ADMINS, u, s)

#define IS_IN_ADMINS 1
#define IS_IN_SURUNNERS 2
#define IS_SPLIT_ADMIN 4
#define IS_TERMINAL_USER 8

DWORD IsInSuRunnersOrAdmins(LPCWSTR DomainAndName, DWORD SessionID);

//  BecomeSuRunner: check, if User is SuRunner if not, try to join him
BOOL BecomeSuRunner(LPCTSTR UserName, DWORD SessionID, bool bIsInAdmins,
                    bool bIsSplitAdmin, BOOL bHimSelf, HWND hwnd);

// User list
//
/////////////////////////////////////////////////////////////////////////////

typedef struct {
  WCHAR UserName[UNLEN + UNLEN + 2];
  HBITMAP UserBitmap;
  SIZE UserBitmapSize;
} USERDATA;

class USERLIST {
public:
  USERLIST();
  ~USERLIST();

public:
  void SetUsualUsers(DWORD SessionId, BOOL bScanDomain);
  void SetGroupUsers(LPWSTR GroupName, DWORD SessionId, BOOL bScanDomain);
  void SetGroupUsers(DWORD WellKnownGroup, DWORD SessionId, BOOL bScanDomain);
  void SetSurunnersUsers(LPCTSTR CurUser, DWORD SessionId, BOOL bScanDomain);
  int FindUser(LPTSTR UserName);
  LPTSTR GetUserName(int nUser);
  HBITMAP GetUserBitmap(int nUser);
  HBITMAP GetUserBitmap(LPTSTR UserName);
  SIZE GetUserBitmapSize(int nUser);
  SIZE GetUserBitmapSize(LPTSTR UserName);
  int GetCount() { return nUsers; };
  void Add(LPCWSTR UserName);
  void DeleteUserBitmap(int i);
  void DeleteUserBitmaps();
  void LoadUserBitmap(int i);
  void LoadUserBitmaps();

private:
  int nUsers;
  USERDATA *User;
  bool m_bSkipAdmins;
  void AddGroupUsers(LPWSTR GroupName, BOOL bScanDomain);
  void AddGroupUsers(DWORD WellKnownGroup, BOOL bScanDomain);
  void AddAllUsers(BOOL bScanDomain);
};
