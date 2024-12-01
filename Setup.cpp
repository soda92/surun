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
#define _DEBUGSETUP
#endif _DEBUG

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include "Setup.h"
#include "Anchor.h"
#include "DBGTrace.h"
#include "Helpers.h"
#include "LSALogon.h"
#include "ResStr.h"
#include "Resource.h"
#include "UserGroups.h"
#include "WinStaDesk.h"
#include "lsa_laar.h"
#include <WinCrypt.h>
#include <commctrl.h>
#include <lm.h>
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>


#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////////////
//
//  Password cache
//
//////////////////////////////////////////////////////////////////////////////

static BYTE KEYPASS[16] = {0x5B, 0xC3, 0x25, 0xE9, 0x8F, 0x2A, 0x41, 0x10,
                           0xA3, 0xF4, 0x26, 0xD1, 0x62, 0xB4, 0x0A, 0xE2};

void LoadPassword(LPTSTR UserName, LPTSTR Password, DWORD nBytes) {
  DWORD Type = REG_BINARY;
  BYTE *ePassword;
  DWORD nB = 0;
  if (!GetRegAnyAlloc(HKLM, PASSWKEY, UserName, &Type, &ePassword, &nB))
    return;
  if (!ePassword)
    return;
  DATA_BLOB PW = {0};
  DATA_BLOB entropy = {sizeof(KEYPASS), KEYPASS};
  DATA_BLOB pw = {nB, (BYTE *)ePassword};
  if (!CryptUnprotectData(&pw, 0, &entropy, 0, 0, CRYPTPROTECT_UI_FORBIDDEN,
                          &PW))
    DBGTrace1("CryptUnprotectData failed: %s", GetLastErrorNameStatic());
  free(ePassword);
  if (PW.pbData) {
    memcpy(Password, PW.pbData, min(nBytes, PW.cbData));
    LocalFree(PW.pbData);
  }
}

void DeletePassword(LPTSTR UserName) {
  RegDelVal(HKLM, PASSWKEY, UserName);
  RegDelVal(HKLM, TIMESKEY, UserName);
}

void SavePassword(LPTSTR UserName, LPTSTR Password) {
  DATA_BLOB pw = {0};
  DATA_BLOB entropy = {sizeof(KEYPASS), KEYPASS};
  DATA_BLOB PW = {(DWORD)(_tcslen(Password) + 1) * sizeof(TCHAR),
                  (BYTE *)Password};
  if (!CryptProtectData(&PW, _T("SuRunUPW"), &entropy, 0, 0,
                        CRYPTPROTECT_UI_FORBIDDEN, &pw))
    DBGTrace1("CryptProtectData failed: %s", GetLastErrorNameStatic());
  if (pw.cbData) {
    SetRegAny(HKLM, PASSWKEY, UserName, REG_BINARY, pw.pbData, pw.cbData);
    LocalFree(pw.pbData);
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//  Password TimeOut
//
//////////////////////////////////////////////////////////////////////////////

// FILETIME(100ns) to minutes multiplier
#define ft2min (__int64)(10 /*1µs*/ * 1000 /*1ms*/ * 1000 /*1s*/ * 60 /*1min*/)

BOOL PasswordExpired(LPTSTR UserName) {
  if ((UserName == 0) || (*UserName == 0))
    return TRUE;
  __int64 ft;
  GetSystemTimeAsFileTime((LPFILETIME)&ft);
  __int64 pwto = ft2min * GetPwTimeOut;
  if ((pwto == 0) || ((ft - GetRegInt64(HKLM, TIMESKEY, UserName, 0)) < pwto))
    return FALSE;
  DeletePassword(UserName);
  return TRUE;
}

void UpdLastRunTime(LPTSTR UserName) {
  __int64 ft;
  GetSystemTimeAsFileTime((LPFILETIME)&ft);
  SetRegInt64(HKLM, TIMESKEY, UserName, ft);
}

//////////////////////////////////////////////////////////////////////////////
//
// WhiteList handling
//
//////////////////////////////////////////////////////////////////////////////

// Common for GetWhiteListFlags and GetBlackListFlags
DWORD GetRegListFlagsAndCmd(HKEY HKR, LPCTSTR SubKey, LPCTSTR CmdLine,
                            LPTSTR cmd, DWORD Default) {
  cmd[0] = 0;
  HKEY Key;
  if (RegOpenKeyEx(HKR, SubKey, 0, KSAM(KEY_READ), &Key) != ERROR_SUCCESS)
    return Default;
  DWORD sizd = sizeof(DWORD);
  DWORD d = Default;
  DWORD t = REG_DWORD;
  if ((RegQueryValueEx(Key, CmdLine, 0, &t, (BYTE *)&d, &sizd) ==
       ERROR_SUCCESS) &&
      (t == REG_DWORD))
    return _tcscpy(cmd, CmdLine), RegCloseKey(Key), d;
  DWORD ccMax = 4096;
  sizd = sizeof(DWORD);
  for (int i = 0; (RegEnumValue(Key, i, cmd, &ccMax, 0, &t, (BYTE *)&d,
                                &sizd) == ERROR_SUCCESS);
       i++) {
    if ((t == REG_DWORD) && strwldcmp(CmdLine, cmd))
      return RegCloseKey(Key), d;
    ccMax = 4096;
    sizd = sizeof(DWORD);
  }
  return RegCloseKey(Key), Default;
}

// Common for GetWhiteListFlags and GetBlackListFlags
DWORD GetRegListFlags(HKEY HKR, LPCTSTR SubKey, LPCTSTR CmdLine,
                      DWORD Default) {
  TCHAR cmd[4096];
  return GetRegListFlagsAndCmd(HKR, SubKey, CmdLine, cmd, Default);
}

DWORD GetCommonWhiteListFlags(LPCTSTR User, LPCTSTR CmdLine, DWORD Default) {
  DWORD dwRet = Default;
  if (GetInstallDevs(User)) {
    // check for new hardware wizard:
    TCHAR s0[MAX_PATH];
    TCHAR s1[MAX_PATH];
    GetSystemWindowsDirectory(s1, 4096);
    _tcscpy(s0, s1);
    PathAppend(s0, L"system32\\rundll32.exe newdev.dll,*");
    if (strwldcmp(CmdLine, s0))
      dwRet = FLAG_DONTASK | FLAG_SHELLEXEC | FLAG_NEVERASK;
    else {
      _tcscpy(s0, s1);
      PathAppend(s0, L"system32\\rundll32.exe ");
      PathAppend(s1, L"system32\\newdev.dll,*");
      _tcscat(s0, s1);
      if (strwldcmp(CmdLine, s0))
        dwRet = FLAG_DONTASK | FLAG_SHELLEXEC | FLAG_NEVERASK;
    }
  }
  return dwRet;
}

DWORD GetWhiteListFlags(LPCTSTR User, LPCTSTR CmdLine, DWORD Default) {
  DWORD dwRet = GetRegListFlags(HKLM, WHTLSTKEY(User), CmdLine, Default);
  if (dwRet != Default)
    return dwRet;
  // Add known Windows stuff
  dwRet = GetCommonWhiteListFlags(User, CmdLine, Default);
  return dwRet;
}

BOOL IsInWhiteList(LPCTSTR User, LPCTSTR CmdLine, DWORD Flag) {
  return (GetWhiteListFlags(User, CmdLine, 0) & Flag) == Flag;
}

BOOL AddToWhiteList(LPCTSTR User, LPCTSTR CmdLine, DWORD Flags /*=0*/) {
  CBigResStr key(_T("%s\\%s"), SVCKEY, User);
  DWORD d = GetRegInt(HKLM, key, CmdLine, (DWORD)-1);
  return (d == Flags) || SetRegInt(HKLM, key, CmdLine, Flags);
}

BOOL SetWhiteListFlag(LPCTSTR User, LPCTSTR CmdLine, DWORD Flag, bool Enable) {
  DWORD d0 = GetWhiteListFlags(User, CmdLine, 0);
  DWORD d1 = (d0 & (~Flag)) | (Enable ? Flag : 0);
  // only save reg key, when flag is different!
  return (d1 == d0) || SetRegInt(HKLM, WHTLSTKEY(User), CmdLine, d1);
}

BOOL ToggleWhiteListFlag(LPCTSTR User, LPCTSTR CmdLine, DWORD Flag) {
  DWORD d = GetWhiteListFlags(User, CmdLine, 0);
  if (d & Flag) {
    d &= ~Flag;
    if (Flag == FLAG_DONTASK) {
      if (d & FLAG_NEVERASK) {
        d &= ~FLAG_NEVERASK;
        d |= FLAG_AUTOCANCEL;
      } else
        d |= FLAG_DONTASK | FLAG_NEVERASK;
    } else if (Flag == FLAG_SHELLEXEC)
      d |= FLAG_CANCEL_SX;
  } else {
    if ((Flag == FLAG_DONTASK) && (d & FLAG_AUTOCANCEL))
      d &= ~FLAG_AUTOCANCEL;
    else if ((Flag == FLAG_SHELLEXEC) && (d & FLAG_CANCEL_SX))
      d &= ~FLAG_CANCEL_SX;
    else
      d |= Flag;
  }
  return SetRegInt(HKLM, WHTLSTKEY(User), CmdLine, d);
}

BOOL RemoveFromWhiteList(LPCTSTR User, LPCTSTR CmdLine) {
  return RegDelVal(HKLM, WHTLSTKEY(User), CmdLine);
}

//////////////////////////////////////////////////////////////////////////////
//
//  BlackList for IATHook
//
//////////////////////////////////////////////////////////////////////////////
DWORD GetBlackListFlags(LPCTSTR CmdLine, DWORD Default) {
  return GetRegListFlags(HKLM, HKLSTKEY, CmdLine, Default);
}

BOOL IsInBlackList(LPCTSTR CmdLine) {
  BOOL blf = GetBlackListFlags(CmdLine, (DWORD)-1);
  if (blf != -1)
    return TRUE;
  TCHAR s[4096] = {0};
  if (GetShortPathName(CmdLine, s, 4096) && (_tcscmp(s, CmdLine))) {
    blf = GetBlackListFlags(s, (DWORD)-1);
    if (blf != -1)
      return TRUE;
  }
  zero(s);
  if (GetLongPathName(CmdLine, s, 4096) && (_tcscmp(s, CmdLine))) {
    blf = GetBlackListFlags(s, (DWORD)-1);
    if (blf != -1)
      return TRUE;
  }
  return FALSE;
}

BOOL AddToBlackList(LPCTSTR CmdLine) {
  return SetRegInt(HKLM, HKLSTKEY, CmdLine, 1);
}

BOOL RemoveFromBlackList(LPCTSTR CmdLine) {
  return RegDelVal(HKLM, HKLSTKEY, CmdLine);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Setup Dialog Data
//
//////////////////////////////////////////////////////////////////////////////
#define nTabs 4
typedef struct _SETUPDATA {
  USERLIST Users;
  LPCTSTR OrgUser;
  DWORD SessID;
  int CurUser;
  HICON UserIcon;
  HWND hTabCtrl[nTabs];
  HWND HelpWnd;
  int DlgExitCode;
  HIMAGELIST ImgList;
  int ImgIconIdx[9];
  TCHAR NewUser[2 * UNLEN + 2];
  CDlgAnchor MainSetupAnchor;
  CDlgAnchor Setup2Anchor;
  int MinW;
  int MinH;
  _SETUPDATA(DWORD SessionID, LPCTSTR User) {
    MinW = 600;
    MinH = 400;
    OrgUser = User;
    SessID = SessionID;
    Users.SetSurunnersUsers(User, SessionID, FALSE);
    CurUser = -1;
    int i;
    for (i = 0; i < Users.GetCount(); i++)
      if (_tcsicmp(Users.GetUserName(i), OrgUser) == 0) {
        CurUser = i;
        break;
      }
    DlgExitCode = IDCANCEL;
    HelpWnd = 0;
    zero(hTabCtrl);
    zero(NewUser);
    UserIcon =
        (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAINICON),
                         IMAGE_ICON, 48, 48, 0);
    ImgList = ImageList_Create(16, 16, ILC_COLOR8, 7, 1);
    for (i = 0; i < 9; i++) {
      HICON icon = (HICON)LoadImage(GetModuleHandle(0),
                                    MAKEINTRESOURCE(IDI_LISTICON + i),
                                    IMAGE_ICON, 0, 0, 0);
      ImgIconIdx[i] = ImageList_AddIcon(ImgList, icon);
      DestroyIcon(icon);
    }
  }
  ~_SETUPDATA() {
    DestroyIcon(UserIcon);
    ImageList_Destroy(ImgList);
  }
} SETUPDATA;

// There can be only one Setup per Application. It's data is stored in g_SD
static SETUPDATA *g_SD = NULL;

//////////////////////////////////////////////////////////////////////////////
//
//  Registry replace stuff
//
//////////////////////////////////////////////////////////////////////////////
void ReplaceRunAsWithSuRun(HKEY hKey /*=HKCR*/) {
  SetHandleRunAs(TRUE);
  TCHAR s[512];
  DWORD i, nS;
  for (i = 0, nS = 512; 0 == RegEnumKey(hKey, i, s, nS); nS = 512, i++) {
    if (_tcsicmp(s, L"CLSID") == 0) {
      HKEY h;
      if (ERROR_SUCCESS == RegOpenKeyEx(hKey, s, 0, KSAM(KEY_ALL_ACCESS), &h))
        ReplaceRunAsWithSuRun(h);
    } else {
      TCHAR v[4096] = {0};
      DWORD n = 4096;
      DWORD t = 0;
      BOOL bRunAsOk =
          GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas\\command"),
                       L"", &t, (BYTE *)&v, &n);
      if (bRunAsOk && ((t == REG_SZ) || (t == REG_EXPAND_SZ))) {
        BOOL bRunAsUserOk = GetRegAnyPtr(
            hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"), L"", &t, 0, 0);
        if (bRunAsUserOk || (_winmajor <= 5)) {
          // Set SuRun command name
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"), L"",
                    CResStr(IDS_RUNAS));
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"), L"Icon",
                    L"SuRunExt.dll,1");
          // Set command
          TCHAR cmd[4096];
          GetSystemWindowsDirectory(cmd, 4096);
          PathAppend(cmd, L"SuRun.exe");
          PathQuoteSpaces(cmd);
          _tcscat(cmd, L" /RUNAS ");
          _tcscat(cmd, v);
          SetRegAny(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun\\command"),
                    L"", t, (BYTE *)&cmd, (DWORD)_tcslen(cmd) * sizeof(TCHAR));
          if (GetRegAnyPtr(hKey,
                           CResStr(L"%s\\%s", s,
                                   (_winmajor <= 5) ? L"shell\\runas"
                                                    : L"shell\\runasuser"),
                           L"Extended", &t, 0, 0))
            SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"),
                      L"Extended", L"");
        }
        // Disable Context menu entries
        if (bRunAsUserOk &&
            (!GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                           L"LegacyDisable", &t, 0, 0))) {
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                    L"LegacyDisable", L"");
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                    L"SR_LegacyDisableWasOff", L"1");
        }
        if ((_winmajor <=
             5) /*WinVista++ use RunAs to elevate and RunAsUser to "Run as"*/
            && (!GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                              L"LegacyDisable", &t, 0, 0))) {
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"LegacyDisable", L"");
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"SR_LegacyDisableWasOff", L"1");
        }
        // Enable SuRun on Entry?
        if (!GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\SuRun\\command"),
                          L"", &t, NULL, 0)) {
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"SR_SuRunWasOff", L"1");
          // Set SuRun command name
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\SuRun"), L"",
                    CResStr(IDS_MENUSTR));
          SetRegStr(hKey, CResStr(L"%s\\%s", s, L"shell\\SuRun"), L"Icon",
                    L"SuRunExt.dll,1");
          // Set command
          TCHAR cmd[4096];
          GetSystemWindowsDirectory(cmd, 4096);
          PathAppend(cmd, L"SuRun.exe");
          PathQuoteSpaces(cmd);
          _tcscat(cmd, L" ");
          _tcscat(cmd, v);
          SetRegAny(hKey, CResStr(L"%s\\%s", s, L"shell\\SuRun\\command"), L"",
                    t, (BYTE *)&cmd, (DWORD)_tcslen(cmd) * sizeof(TCHAR));
        }
      }
    }
  }
}

void ReplaceSuRunWithRunAs(HKEY hKey /*=HKCR*/) {
  SetHandleRunAs(FALSE);
  TCHAR s[512];
  DWORD i, nS;
  for (i = 0, nS = 512; 0 == RegEnumKey(hKey, i, s, nS); nS = 512, i++) {
    if (_tcsicmp(s, L"CLSID") == 0) {
      HKEY h;
      if (ERROR_SUCCESS == RegOpenKeyEx(hKey, s, 0, KSAM(KEY_ALL_ACCESS), &h))
        ReplaceSuRunWithRunAs(h);
    } else {
      TCHAR v[4096];
      DWORD n = 4096;
      DWORD t = 0;
      if (GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"),
                       L"SuRunWasHere", &t, (BYTE *)&v, &n) &&
          ((t == REG_SZ) || (t == REG_EXPAND_SZ))) {
        // Old RunAs method, RegRename...
        RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"),
                  L"SuRunWasHere");
        RenameRegKey(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"),
                     CResStr(L"%s\\%s", s, L"shell\\runas"));
        // Restore original command
        SetRegAny(hKey, CResStr(L"%s\\%s", s, L"shell\\runas\\command"), L"", t,
                  (BYTE *)&v, (DWORD)_tcslen(v) * sizeof(TCHAR));
        // Restore  original command name:
        n = 4096;
        zero(v);
        if (GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                         L"orgname", &t, (BYTE *)&v, &n)) {
          if (v[0])
            SetRegAny(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"), L"", t,
                      (BYTE *)&v, n);
          else
            RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"), 0);
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"), L"orgname");
        }
      } else // new: use LegacyDisable
      {
        if (GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                         L"SR_SuRunWasOff", &t, 0, 0)) {
          DelRegKey(hKey, CResStr(L"%s\\%s", s, L"shell\\SuRun"));
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"SR_SuRunWasOff");
        }
        if (GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                         L"SR_LegacyDisableWasOff", &t, 0, 0)) {
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"LegacyDisable");
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runas"),
                    L"SR_LegacyDisableWasOff");
        }
        if (GetRegAnyPtr(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                         L"SR_LegacyDisableWasOff", &t, 0, 0)) {
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                    L"LegacyDisable");
          RegDelVal(hKey, CResStr(L"%s\\%s", s, L"shell\\runasuser"),
                    L"SR_LegacyDisableWasOff");
        }
        DelRegKey(hKey, CResStr(L"%s\\%s", s, L"shell\\RunAsSuRun"));
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//  Select a User Dialog
//
//////////////////////////////////////////////////////////////////////////////
static int CALLBACK UsrListSortProc(LPARAM lParam1, LPARAM lParam2,
                                    LPARAM lParamSort) {
  TCHAR s1[MAX_PATH];
  TCHAR s2[MAX_PATH];
  ListView_GetItemText((HWND)lParamSort, lParam1, 0, s1, MAX_PATH);
  ListView_GetItemText((HWND)lParamSort, lParam2, 0, s2, MAX_PATH);
  return _tcsicmp(s1, s2);
}

static void SetSelectedNameText(HWND hwnd) {
  HWND hUL = GetDlgItem(hwnd, IDC_USERLIST);
  int n = ListView_GetSelectionMark(hUL);
  if (n >= 0) {
    TCHAR u[MAX_PATH] = {0};
    ListView_GetItemText(hUL, n, 0, u, MAX_PATH);
    if (u[0])
      SetDlgItemText(hwnd, IDC_USERNAME, u);
  }
}

static void AddUsers(HWND hwnd, BOOL bDomainUsers) {
  HWND hUL = GetDlgItem(hwnd, IDC_USERLIST);
  USERLIST ul;
  ul.SetGroupUsers(L"*", g_SD->SessID, bDomainUsers);
  ListView_DeleteAllItems(hUL);
  for (int i = 0; i < ul.GetCount(); i++) {
    LVITEM item = {LVIF_TEXT, i, 0, 0, 0, ul.GetUserName(i), 0, 0, 0, 0};
    if (!IsInSuRunners(item.pszText, g_SD->SessID))
      ListView_InsertItem(hUL, &item);
  }
  ListView_SortItemsEx(hUL, UsrListSortProc, hUL);
  ListView_SetColumnWidth(hUL, 0, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetItemState(hUL, 0, LVIS_FOCUSED | LVIS_SELECTED,
                        LVIS_FOCUSED | LVIS_SELECTED);
  SetSelectedNameText(hwnd);
  SetFocus(GetDlgItem(hwnd, IDC_USERNAME));
  SendMessage(GetDlgItem(hwnd, IDC_USERNAME), EM_SETSEL, 0, -1);
}

INT_PTR CALLBACK SelUserDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  switch (msg) {
  case WM_INITDIALOG: {
    BringToPrimaryMonitor(hwnd);
    HWND hUL = GetDlgItem(hwnd, IDC_USERLIST);
    SendMessage(hUL, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_INFOTIP);
    LVCOLUMN col = {LVCF_WIDTH, 0, 22, 0, 0, 0, 0, 0};
    ListView_InsertColumn(hUL, 0, &col);
    AddUsers(hwnd, FALSE);
  }
    return FALSE;
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_NOTIFY: {
    switch (wParam) {
    // Program List Notifications
    case IDC_USERLIST:
      if (lParam)
        switch (((LPNMHDR)lParam)->code) {
        // Mouse Click
        case NM_DBLCLK:
          SetSelectedNameText(hwnd);
          GetDlgItemText(hwnd, IDC_USERNAME, g_SD->NewUser,
                         countof(g_SD->NewUser));
          EndDialog(hwnd, 1);
          return TRUE;
        case NM_CLICK:
        // Selection changed
        case LVN_ITEMCHANGED:
          SetSelectedNameText(hwnd);
          return TRUE;
        } // switch (switch(((LPNMHDR)lParam)->code)
    } // switch (wParam)
    break;
  } // WM_NOTIFY
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(IDC_ALLUSERS, BN_CLICKED):
      AddUsers(hwnd, IsDlgButtonChecked(hwnd, IDC_ALLUSERS));
      return TRUE;
    case MAKELPARAM(IDCANCEL, BN_CLICKED):
      EndDialog(hwnd, 0);
      return TRUE;
    case MAKELPARAM(IDOK, BN_CLICKED):
      GetDlgItemText(hwnd, IDC_USERNAME, g_SD->NewUser, countof(g_SD->NewUser));
      EndDialog(hwnd, 1);
      return TRUE;
    } // switch (wParam)
    break;
  } // WM_COMMAND
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
//  Service setup
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Add/Edit file to users file list:
//
//////////////////////////////////////////////////////////////////////////////
struct {
  DWORD *Flags;
  LPTSTR FileName;
  int OfnTitle;
} g_AppOpt;

static BOOL ChooseFile(HWND hwnd, LPTSTR FileName) {
  CImpersonateSessionUser ilu(g_SD->SessID);
  OPENFILENAME ofn = {0};
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = TEXT("*.*\0*.*\0\0");
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = FileName;
  ofn.nMaxFile = 4096;
  ofn.lpstrTitle = CResStr(g_AppOpt.OfnTitle);
  ofn.Flags = OFN_ENABLESIZING | OFN_NOVALIDATE | OFN_FORCESHOWHIDDEN;
  ofn.FlagsEx = OFN_EX_NOPLACESBAR;
  BOOL bRet = GetOpenFileName(&ofn);
  if (PathFileExists(FileName))
    PathQuoteSpaces(FileName);
  return bRet;
}

INT_PTR CALLBACK AppOptDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (msg) {
  case WM_INITDIALOG:
    BringToPrimaryMonitor(hwnd);
    SetDlgItemText(hwnd, IDC_FILENAME, g_AppOpt.FileName);
    if (g_AppOpt.OfnTitle == IDS_ADDFILETOLIST) {
      CheckDlgButton(hwnd, IDC_NOASK1,
                     (*g_AppOpt.Flags &
                      (FLAG_DONTASK | FLAG_NEVERASK | FLAG_AUTOCANCEL)) == 0);
      CheckDlgButton(hwnd, IDC_NOASK2,
                     (*g_AppOpt.Flags & (FLAG_DONTASK | FLAG_NEVERASK)) ==
                         FLAG_DONTASK);
      CheckDlgButton(hwnd, IDC_NOASK4,
                     (*g_AppOpt.Flags & (FLAG_DONTASK | FLAG_NEVERASK)) ==
                         (FLAG_DONTASK | FLAG_NEVERASK));
      CheckDlgButton(hwnd, IDC_NOASK3,
                     (*g_AppOpt.Flags & FLAG_AUTOCANCEL) != 0);

      CheckDlgButton(hwnd, IDC_AUTO1,
                     (*g_AppOpt.Flags & (FLAG_SHELLEXEC | FLAG_CANCEL_SX)) ==
                         0);
      CheckDlgButton(hwnd, IDC_AUTO2, (*g_AppOpt.Flags & FLAG_SHELLEXEC) != 0);
      CheckDlgButton(hwnd, IDC_AUTO3, (*g_AppOpt.Flags & FLAG_CANCEL_SX) != 0);

      if ((!IsDlgButtonChecked(g_SD->hTabCtrl[2], IDC_SHEXHOOK)) &&
          (!IsDlgButtonChecked(g_SD->hTabCtrl[2], IDC_IATHOOK))) {
        EnableWindow(GetDlgItem(hwnd, IDC_AUTO1), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_AUTO2), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_AUTO3), 0);
      }
    }
    return TRUE;
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_COMMAND:
    switch (wParam) {
    case MAKELPARAM(IDC_SELFILE, BN_CLICKED):
      GetDlgItemText(hwnd, IDC_FILENAME, g_AppOpt.FileName, 4096);
      ChooseFile(hwnd, g_AppOpt.FileName);
      if (g_AppOpt.OfnTitle != IDS_ADDFILETOLIST)
        PathUnquoteSpaces(g_AppOpt.FileName);
      SetDlgItemText(hwnd, IDC_FILENAME, g_AppOpt.FileName);
      break;
    case MAKELPARAM(IDCANCEL, BN_CLICKED):
      EndDialog(hwnd, IDCANCEL);
      return TRUE;
    case MAKELPARAM(IDOK, BN_CLICKED):
      GetDlgItemText(hwnd, IDC_FILENAME, g_AppOpt.FileName, 4096);
      if (g_AppOpt.OfnTitle == IDS_ADDFILETOLIST) {
        // Test drive:
        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {0};
        if (CreateProcess(NULL, g_AppOpt.FileName, NULL, NULL, FALSE,
                          CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, 0,
                          NULL, &si, &pi)) {
          TerminateProcess(pi.hProcess, 0);
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
        } else if (SafeMsgBox(hwnd, CBigResStr(IDS_APPOK), CResStr(IDS_APPNAME),
                              MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) ==
                   IDNO)
          return TRUE;
        *g_AppOpt.Flags = 0;
        if (IsDlgButtonChecked(hwnd, IDC_NOASK2))
          *g_AppOpt.Flags |= FLAG_DONTASK;
        if (IsDlgButtonChecked(hwnd, IDC_NOASK3))
          *g_AppOpt.Flags |= FLAG_AUTOCANCEL;
        if (IsDlgButtonChecked(hwnd, IDC_NOASK4))
          *g_AppOpt.Flags |= FLAG_DONTASK | FLAG_NEVERASK;
        if (IsDlgButtonChecked(hwnd, IDC_AUTO2))
          *g_AppOpt.Flags |= FLAG_SHELLEXEC;
        if (IsDlgButtonChecked(hwnd, IDC_AUTO3))
          *g_AppOpt.Flags |= FLAG_CANCEL_SX;
      }
      EndDialog(hwnd, IDOK);
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL GetFileName(HWND hwnd, DWORD &Flags, LPTSTR FileName,
                        int DlgID = IDD_APPOPTIONS,
                        int OfnTitle = IDS_ADDFILETOLIST) {
  if ((DlgID == IDD_APPOPTIONS) && (FileName[0] == 0)) {
    if (!ChooseFile(hwnd, FileName))
      return FALSE;
  }
  g_AppOpt.FileName = FileName;
  g_AppOpt.Flags = &Flags;
  g_AppOpt.OfnTitle = OfnTitle;
  return DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(DlgID), hwnd,
                   AppOptDlgProc) == IDOK;
}

//////////////////////////////////////////////////////////////////////////////
//
// Add/Edit file to IAT-Hook Blacklist:
//
//////////////////////////////////////////////////////////////////////////////
static void FillBlackList(HWND hwnd) {
  HWND hBL = GetDlgItem(hwnd, IDC_BLACKLIST);
  ListView_DeleteAllItems(hBL);
  HKEY Key;
  if (RegOpenKeyEx(HKLM, HKLSTKEY, 0, KSAM(KEY_READ), &Key) == ERROR_SUCCESS) {
    TCHAR cmd[4096];
    DWORD ccMax = countof(cmd);
    for (int i = 0;
         (RegEnumValue(Key, i, cmd, &ccMax, 0, 0, 0, 0) == ERROR_SUCCESS);
         i++) {
      ccMax = countof(cmd);
      LVITEM item = {LVIF_TEXT, i, 0, 0, 0, cmd, 0, 0, 0, 0};
      ListView_InsertItem(hBL, &item);
    }
    RegCloseKey(Key);
  }
  ListView_SortItemsEx(hBL, UsrListSortProc, hBL);
  ListView_SetColumnWidth(hBL, 0, LVSCW_AUTOSIZE_USEHEADER);
  ListView_SetItemState(hBL, 0, LVIS_FOCUSED | LVIS_SELECTED,
                        LVIS_FOCUSED | LVIS_SELECTED);
  int cSel = ListView_GetSelectedCount(hBL);
  EnableWindow(GetDlgItem(hwnd, IDC_DELETE), cSel >= 1);
  EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), cSel == 1);
}

INT_PTR CALLBACK BlkLstDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  switch (msg) {
  case WM_INITDIALOG: {
    BringToPrimaryMonitor(hwnd);
    HWND hBL = GetDlgItem(hwnd, IDC_BLACKLIST);
    SendMessage(hBL, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_INFOTIP);
    LVCOLUMN col = {LVCF_WIDTH, 0, 22, 0, 0, 0, 0, 0};
    ListView_InsertColumn(hBL, 0, &col);
    FillBlackList(hwnd);
  }
    return TRUE;
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_COMMAND:
    switch (wParam) {
    case MAKELPARAM(IDC_ADDAPP, BN_CLICKED): {
      TCHAR cmd[4096] = {0};
      DWORD f = 0;
      if (GetFileName(hwnd, f, cmd, IDD_ADDTOBLKLST, IDS_ADDTOBLKLIST) &&
          cmd[0]) {
        AddToBlackList(cmd);
        FillBlackList(hwnd);
      }
    }
      return TRUE;
    // Edit App Button
    case MAKELPARAM(IDC_EDITAPP, BN_CLICKED): {
    EditApp:
      HWND hBL = GetDlgItem(hwnd, IDC_BLACKLIST);
      int nSel = ListView_GetSelectionMark(hBL);
      int cSel = ListView_GetSelectedCount(hBL);
      if ((nSel >= 0) && (cSel == 1)) {
        TCHAR cmd[4096];
        TCHAR CMD[4096];
        ListView_GetItemText(hBL, nSel, 0, cmd, 4096);
        _tcscpy(CMD, cmd);
        DWORD f = 0;
        if (GetFileName(hwnd, f, CMD, IDD_ADDTOBLKLST, IDS_ADDTOBLKLIST) &&
            (CMD[0]) && (_tcsicmp(CMD, cmd) != 0)) {
          if ((GetRegInt(HKLM, HKLSTKEY, CMD, (DWORD)-1) ==
               (DWORD)-1) // No duplicates!
              && RemoveFromBlackList(cmd)) {
            AddToBlackList(CMD);
            FillBlackList(hwnd);
          } else
            MessageBeep(MB_ICONERROR);
        }
      }
    }
      return TRUE;
    // Delete App Button
    case MAKELPARAM(IDC_DELETE, BN_CLICKED): {
    DelApp:
      HWND hBL = GetDlgItem(hwnd, IDC_BLACKLIST);
      int cSel = ListView_GetSelectedCount(hBL);
      if (cSel > 0)
        for (int n = 0; n < ListView_GetItemCount(hBL);) {
          if (ListView_GetItemState(hBL, n, LVIS_SELECTED)) {
            TCHAR cmd[4096];
            ListView_GetItemText(hBL, n, 0, cmd, 4096);
            if (RemoveFromBlackList(cmd)) {
              ListView_DeleteItem(hBL, n);
            } else
              MessageBeep(MB_ICONERROR);
          } else
            n++;
        }
      int nSel = ListView_GetSelectionMark(hBL);
      if (nSel >= 0)
        ListView_SetItemState(hBL, nSel, LVIS_SELECTED, 0x0F);
      EnableWindow(GetDlgItem(hwnd, IDC_DELETE), nSel >= 0);
      EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), nSel >= 0);
    }
      return TRUE;
    case MAKELPARAM(IDCANCEL, BN_CLICKED):
    case MAKELPARAM(IDOK, BN_CLICKED):
      EndDialog(hwnd, IDCANCEL);
      return TRUE;
    } // WM_COMMAND
  case WM_NOTIFY: {
    switch (wParam) {
    // Program List Notifications
    case IDC_BLACKLIST:
      if (lParam)
        switch (((LPNMHDR)lParam)->code) {
        case LVN_KEYDOWN:
          switch (((LPNMLVKEYDOWN)lParam)->wVKey) {
          case VK_F2:
            goto EditApp;
          case VK_DELETE:
            goto DelApp;
          case (WORD)'A':
            if (GetKeyState(VK_CONTROL) & 0x8000) {
              HWND hBL = GetDlgItem(hwnd, IDC_BLACKLIST);
              if (ListView_GetItemCount(hBL)) {
                for (int n = 0; n < ListView_GetItemCount(hBL); n++)
                  ListView_SetItemState(hBL, n, LVIS_SELECTED, 0x0F);
                ListView_SetItemState(hBL, 0, LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);
              }
              return TRUE;
            }
          }
          break;
        // Mouse Click: Toggle Flags
        case NM_DBLCLK:
          goto EditApp;
        // Selection changed
        case LVN_ITEMCHANGED:
          EnableWindow(
              GetDlgItem(hwnd, IDC_DELETE),
              ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_BLACKLIST)) >= 1);
          EnableWindow(
              GetDlgItem(hwnd, IDC_EDITAPP),
              ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_BLACKLIST)) == 1);
          return TRUE;
        } // switch (switch(((LPNMHDR)lParam)->code)
    } // switch (wParam)
    break;
  } // WM_NOTIFY
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Save Flags for the selected User
//
//////////////////////////////////////////////////////////////////////////////
static void SaveUserFlags() {
  if (g_SD->CurUser >= 0) {
    LPTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
    SetNoRunSetup(u, IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_RUNSETUP) == 0);
    SetRestrictApps(u,
                    IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_RESTRICTED) != 0);
    if (GetUseSVCHook)
      SetInstallDevs(u,
                     IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_HW_ADMIN) != 0);
    SetHideFromUser(u,
                    IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_HIDESURUN) != 0);
    SetReqPw4Setup(u,
                   IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_REQPW4SETUP) != 0);
    SetStoreUsrPW(u, IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_STORE_PW) != 0);
    if (!IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_TRAYSHOWADMIN)) {
      SetUserTSA(u, 0);
    } else {
      SetUserTSA(
          u, 1 + (DWORD)IsDlgButtonChecked(g_SD->hTabCtrl[1], IDC_TRAYBALLOON));
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// Program List handling
//
//////////////////////////////////////////////////////////////////////////////
static int CALLBACK ListSortProc(LPARAM lParam1, LPARAM lParam2,
                                 LPARAM lParamSort) {
  TCHAR s1[4096];
  TCHAR s2[4096];
  ListView_GetItemText((HWND)lParamSort, lParam1, 2, s1, 4096);
  ListView_GetItemText((HWND)lParamSort, lParam2, 2, s2, 4096);
  return _tcsicmp(s1, s2);
}

static void UpdateWhiteListFlags(HWND hWL) {
  CBigResStr wlkey(_T("%s\\%s"), SVCKEY,
                   g_SD->Users.GetUserName(g_SD->CurUser));
  TCHAR cmd[4096];
  for (int i = 0; i < ListView_GetItemCount(hWL); i++) {
    ListView_GetItemText(hWL, i, 2, cmd, 4096);
    int Flags = GetRegInt(HKLM, wlkey, cmd, 0);
    LVITEM item = {LVIF_IMAGE,
                   i,
                   0,
                   0,
                   0,
                   0,
                   0,
                   g_SD->ImgIconIdx[2 + (Flags & FLAG_DONTASK ? 1 : 0) +
                                    (Flags & FLAG_NEVERASK ? 5 : 0) +
                                    (Flags & FLAG_AUTOCANCEL ? 4 : 0)],
                   0,
                   0};
    ListView_SetItem(hWL, &item);
    item.iSubItem = 1;
    item.iImage = g_SD->ImgIconIdx[(Flags & FLAG_SHELLEXEC ? 0 : 1) +
                                   (Flags & FLAG_CANCEL_SX ? 6 : 0)];
    ListView_SetItem(hWL, &item);
  }
  ListView_SetColumnWidth(
      hWL, 1,
      (IsDlgButtonChecked(g_SD->hTabCtrl[2], IDC_SHEXHOOK) ||
       IsDlgButtonChecked(g_SD->hTabCtrl[2], IDC_IATHOOK))
          ? 22
          : 0);
  ListView_SetColumnWidth(hWL, 2, LVSCW_AUTOSIZE_USEHEADER);
  InvalidateRect(hWL, 0, TRUE);
}

//////////////////////////////////////////////////////////////////////////////
//
// Populate the Program list for the selected User, show the User Icon
//
//////////////////////////////////////////////////////////////////////////////
static void UpdateUser(HWND hwnd) {
  int n = (int)SendDlgItemMessage(hwnd, IDC_USER, CB_GETCURSEL, 0, 0);
  HBITMAP bm = 0;
  HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
  if ((n >= 0) && (g_SD->CurUser == n))
    return;
  // Save Settings:
  SaveUserFlags();
  g_SD->CurUser = n;
  ListView_DeleteAllItems(hWL);
  if (n != CB_ERR) {
    LPTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
    bm = g_SD->Users.GetUserBitmap(n);
    EnableWindow(GetDlgItem(hwnd, IDC_USER), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_DELUSER), GetUseSuRunGrp);
    EnableWindow(GetDlgItem(hwnd, IDC_IMPORT), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_EXPORT), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_HW_ADMIN), GetUseSVCHook);
    CheckDlgButton(hwnd, IDC_RUNSETUP, !GetNoRunSetup(u));
    EnableWindow(GetDlgItem(hwnd, IDC_RUNSETUP), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_HIDESURUN), 1);
    CheckDlgButton(hwnd, IDC_RESTRICTED, GetRestrictApps(u));
    CheckDlgButton(hwnd, IDC_HW_ADMIN, GetUseSVCHook && GetInstallDevs(u));
    CheckDlgButton(hwnd, IDC_HIDESURUN, GetHideFromUser(u));
    if (GetNoRunSetup(u)) {
      EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP), 0);
      CheckDlgButton(hwnd, IDC_REQPW4SETUP, 0);
    } else {
      EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP), 1);
      CheckDlgButton(hwnd, IDC_REQPW4SETUP, GetReqPw4Setup(u));
    }
    // TSA:
    // Win2k:no balloon tips
    EnableWindow(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), 0);
    CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_TRAYSHOWADMIN, BST_UNCHECKED);
    EnableWindow(GetDlgItem(hwnd, IDC_STORE_PW), 1);
    CheckDlgButton(hwnd, IDC_STORE_PW, GetStoreUsrPW(u));
    switch (GetUserTSA(u)) {
    case 2:
      CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_CHECKED);
    case 1:
      if (!IsWin2k)
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), 1);
      CheckDlgButton(hwnd, IDC_TRAYSHOWADMIN, BST_CHECKED);
      break;
    }
    if (GetHideFromUser(u)) {
      EnableWindow(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN), 0);
      EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), 0);
      EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP), 0);
      EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED), 0);
      EnableWindow(GetDlgItem(hwnd, IDC_HW_ADMIN), GetUseSVCHook);
      EnableWindow(GetDlgItem(hwnd, IDC_RUNSETUP), 0);
      EnableWindow(GetDlgItem(hwnd, IDC_STORE_PW), 0);
      CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_UNCHECKED);
      CheckDlgButton(hwnd, IDC_TRAYSHOWADMIN, BST_UNCHECKED);
      CheckDlgButton(hwnd, IDC_RUNSETUP, BST_UNCHECKED);
      CheckDlgButton(hwnd, IDC_REQPW4SETUP, BST_UNCHECKED);
      CheckDlgButton(hwnd, IDC_RESTRICTED, BST_CHECKED);
      CheckDlgButton(hwnd, IDC_HW_ADMIN, GetUseSVCHook && GetInstallDevs(u));
      CheckDlgButton(hwnd, IDC_RUNSETUP, BST_UNCHECKED);
      CheckDlgButton(hwnd, IDC_STORE_PW, BST_UNCHECKED);
    }
    EnableWindow(hWL, true);
    HKEY Key;
    if (RegOpenKeyEx(HKLM, WHTLSTKEY(u), 0, KSAM(KEY_READ), &Key) ==
        ERROR_SUCCESS) {
      TCHAR cmd[4096];
      DWORD ccMax = countof(cmd);
      for (int i = 0;
           (RegEnumValue(Key, i, cmd, &ccMax, 0, 0, 0, 0) == ERROR_SUCCESS);
           i++) {
        ccMax = countof(cmd);
        LVITEM item = {LVIF_IMAGE, i, 0, 0, 0, 0, 0, g_SD->ImgIconIdx[0], 0, 0};
        ListView_SetItemText(hWL, ListView_InsertItem(hWL, &item), 2, cmd);
      }
      RegCloseKey(Key);
    }

    ListView_SortItemsEx(hWL, ListSortProc, hWL);
    UpdateWhiteListFlags(hWL);

    EnableWindow(GetDlgItem(hwnd, IDC_ADDAPP), 1);
    int cSel = ListView_GetSelectedCount(hWL);
    EnableWindow(GetDlgItem(hwnd, IDC_DELETE), cSel >= 1);
    EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), cSel == 1);
  } else {
    EnableWindow(GetDlgItem(hwnd, IDC_USER), 0);
    EnableWindow(GetDlgItem(hwnd, IDC_DELUSER), false);
    EnableWindow(GetDlgItem(hwnd, IDC_IMPORT), 0);
    EnableWindow(GetDlgItem(hwnd, IDC_EXPORT), 0);
    EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED), false);
    EnableWindow(GetDlgItem(hwnd, IDC_HW_ADMIN), false);
    EnableWindow(GetDlgItem(hwnd, IDC_RUNSETUP), false);
    EnableWindow(GetDlgItem(hwnd, IDC_HIDESURUN), false);
    EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP), false);
    EnableWindow(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN), false);
    EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), false);
    EnableWindow(GetDlgItem(hwnd, IDC_STORE_PW), false);
    EnableWindow(hWL, false);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDAPP), false);
    EnableWindow(GetDlgItem(hwnd, IDC_DELETE), false);
    EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), false);
  }
  HWND BmpIcon = GetDlgItem(hwnd, IDC_USERBITMAP);
  DWORD dwStyle = GetWindowLong(BmpIcon, GWL_STYLE) &
                  (~(SS_TYPEMASK | SS_REALSIZEIMAGE | SS_CENTERIMAGE));
  if (bm) {
    SIZE sz = g_SD->Users.GetUserBitmapSize(n);
    RECT r;
    GetClientRect(BmpIcon, &r);
    if ((r.right - r.left < sz.cx) || (r.bottom - r.top < sz.cy))
      SetWindowLong(BmpIcon, GWL_STYLE,
                    dwStyle | SS_BITMAP | SS_REALSIZECONTROL);
    else
      SetWindowLong(BmpIcon, GWL_STYLE,
                    dwStyle | SS_BITMAP | SS_REALSIZEIMAGE | SS_CENTERIMAGE);
    SendMessage(BmpIcon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bm);
  } else {
    SetWindowLong(BmpIcon, GWL_STYLE,
                  dwStyle | SS_ICON | SS_REALSIZEIMAGE | SS_CENTERIMAGE);
    SendMessage(BmpIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_SD->UserIcon);
  }
  InvalidateRect(hWL, 0, TRUE);
}

//////////////////////////////////////////////////////////////////////////////
//
// Populate User Combobox and the Program list for the selected User
//
//////////////////////////////////////////////////////////////////////////////
static void UpdateUserList(HWND hwnd, LPCTSTR UserName) {
  SendDlgItemMessage(hwnd, IDC_USER, CB_RESETCONTENT, 0, 0);
  SendDlgItemMessage(hwnd, IDC_USER, CB_SETCURSEL, (DWORD)-1, 0);
  g_SD->CurUser = 0;
  for (int i = 0; i < g_SD->Users.GetCount(); i++) {
    LPTSTR u = g_SD->Users.GetUserName(i);
    SendDlgItemMessage(hwnd, IDC_USER, CB_INSERTSTRING, i, (LPARAM)u);
    if (_tcsicmp(u, UserName) == 0)
      g_SD->CurUser = i;
  }
  SendDlgItemMessage(hwnd, IDC_USER, CB_SETCURSEL, g_SD->CurUser, 0);
  g_SD->CurUser = -1;
  UpdateUser(hwnd);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Import/Export
//
//////////////////////////////////////////////////////////////////////////////
BOOL WritePrivateProfileInt(LPCTSTR App, LPCTSTR Key, DWORD Val, LPCTSTR ini) {
  TCHAR s[16] = {0};
  return WritePrivateProfileString(App, Key, _itot(Val, s, 10), ini);
}

#define EXPORTINT(App, f, ini) WritePrivateProfileInt(App, _T(#f), Get##f, ini)
#define IMPORTINT(App, f, ini)                                                 \
  Set##f(GetPrivateProfileInt(App, _T(#f), Get##f, ini))

#define EXPORTINTu(App, f, u, ini)                                             \
  WritePrivateProfileInt(App, _T(#f), Get##f(u), ini)
#define IMPORTINTu(App, f, u, ini)                                             \
  Set##f(u, GetPrivateProfileInt(App, _T(#f), Get##f(u), ini))

#define EXPORTSTR(App, n, s, ini)                                              \
  {                                                                            \
    TCHAR ts[16];                                                              \
    WritePrivateProfileString(App, _itot(n, ts, 10), s, ini);                  \
  }

#define IMPORTSTR(App, n, s, ini)                                              \
  {                                                                            \
    TCHAR ts[16];                                                              \
    GetPrivateProfileString(App, _itot(n, ts, 10), _T(""), s, countof(s),      \
                            ini);                                              \
  }

#define EXPORTVAL(App, n, v, ini)                                              \
  {                                                                            \
    TCHAR ts[16];                                                              \
    WritePrivateProfileInt(App, _itot(n, ts, 10), v, ini);                     \
  }

#define IMPORTVAL(App, n, d, ini)                                              \
  {                                                                            \
    TCHAR ts[16];                                                              \
    d = GetPrivateProfileInt(App, _itot(n, ts, 10), d, ini);                   \
  }

void ExportUserWhiteList(LPCTSTR ini, LPCTSTR User, LPCTSTR WLKEY,
                         LPCTSTR WLFKEY) {
  HKEY Key;
  // Export user list
  if (RegOpenKeyEx(HKLM, WHTLSTKEY(User), 0, KSAM(KEY_READ), &Key) ==
      ERROR_SUCCESS) {
    TCHAR cmd[4096];
    _tcscpy(cmd, _T("\"")); // Quote strings!
    DWORD ccMax = countof(cmd) - 2;
    DWORD Flags = 0;
    DWORD siz = sizeof(Flags);
    for (int i = 0; (RegEnumValue(Key, i, &cmd[1], &ccMax, 0, 0, (BYTE *)&Flags,
                                  &siz) == ERROR_SUCCESS);
         i++) {
      ccMax = countof(cmd);
      siz = sizeof(Flags);
      _tcscat(cmd, _T("\"")); // Quote strings!
      EXPORTSTR(WLKEY, i, cmd, ini);
      EXPORTVAL(WLFKEY, i, Flags, ini);
      _tcscpy(cmd, _T("\"")); // Quote strings!
    }
    RegCloseKey(Key);
  }
}

void ExportUser(LPCTSTR ini, LPCTSTR User, int nUser) {
  CResStr USRKEY(_T("User%d"), nUser);
  CResStr WLKEY(_T("WhiteList%d"), nUser);
  CResStr WLFKEY(_T("WhiteListFlags%d"), nUser);
  // Detect local users:
  // These users will be imported to local users on the target machine
  // Domain users will be imported to domain users on the target machine
  TCHAR cn[UNLEN + 1] = {0};
  DWORD cnl = UNLEN;
  GetComputerName(cn, &cnl);
  TCHAR dn[2 * UNLEN + 1] = {0};
  _tcscpy(dn, User);
  PathRemoveFileSpec(dn);
  BOOL IsLocal = _tcsicmp(dn, cn) == 0;
  _tcscpy(dn, User);
  PathStripPath(dn);
  WritePrivateProfileString(USRKEY, _T("Name"), IsLocal ? dn : User, ini);
  WritePrivateProfileInt(USRKEY, _T("IsLocalUser"), IsLocal, ini);
  // Export user settings
  EXPORTINTu(USRKEY, NoRunSetup, User, ini);
  EXPORTINTu(USRKEY, RestrictApps, User, ini);
  EXPORTINTu(USRKEY, InstallDevs, User, ini);
  EXPORTINTu(USRKEY, UserTSA, User, ini);
  EXPORTINTu(USRKEY, HideFromUser, User, ini);
  EXPORTINTu(USRKEY, ReqPw4Setup, User, ini);
  EXPORTINTu(USRKEY, StoreUsrPW, User, ini);
  ExportUserWhiteList(ini, User, WLKEY, WLFKEY);
}

BOOL ImportUserWhiteList(LPCTSTR ini, LPCTSTR User, LPCTSTR WLKEY,
                         LPCTSTR WLFKEY) {
  CBigResStr key(_T("%s\\%s"), SVCKEY, User);
  for (int i = 0;; i++) {
    TCHAR cmd[4096];
    cmd[0] = 0;
    IMPORTSTR(WLKEY, i, cmd, ini);
    if (cmd[0]) {
      DWORD d = 0;
      IMPORTVAL(WLFKEY, i, d, ini);
      DBGTrace3("%s: %s=%x", WLKEY, cmd, d);
      if (GetRegInt(HKLM, key, cmd, (DWORD)-1) == (DWORD)-1) {
        SetRegInt(HKLM, key, cmd, d);
        DBGTrace3("%s: %s=%x ok", WLKEY, cmd, d);
      }
    } else
      break;
  }
  return TRUE;
}

BOOL ImportUser(LPCTSTR ini, LPCTSTR USRKEY, LPCTSTR WLKEY, LPCTSTR WLFKEY) {
  TCHAR User[2 * UNLEN + 1] = {0};
  GetPrivateProfileString(USRKEY, _T("Name"), NULL, User, countof(User), ini);
  if (!User[0])
    return FALSE;
  BOOL IsLocal = GetPrivateProfileInt(USRKEY, _T("IsLocalUser"), 1, ini);
  if (IsLocal) {
    // Put User name after local computer name
    TCHAR cn[2 * UNLEN + 1] = {0};
    DWORD cnl = 2 * UNLEN;
    GetComputerName(cn, &cnl);
    PathAppend(cn, User);
    _tcscpy(User, cn);
  }
  if (!UACEnabled)
    AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, User, 0);
  AlterGroupMember(DOMAIN_ALIAS_RID_USERS, User, 1);
  if (GetUseSuRunGrp) {
    DWORD dwe = AlterGroupMember(SURUNNERSGROUP, User, 1);
    if (dwe && (dwe != ERROR_MEMBER_IN_ALIAS))
      return FALSE;
  }
  DelUsrSettings(User);
  IMPORTINTu(USRKEY, NoRunSetup, User, ini);
  IMPORTINTu(USRKEY, RestrictApps, User, ini);
  IMPORTINTu(USRKEY, InstallDevs, User, ini);
  IMPORTINTu(USRKEY, UserTSA, User, ini);
  IMPORTINTu(USRKEY, HideFromUser, User, ini);
  IMPORTINTu(USRKEY, ReqPw4Setup, User, ini);
  IMPORTINTu(USRKEY, StoreUsrPW, User, ini);
  return ImportUserWhiteList(ini, User, WLKEY, WLFKEY);
}

BOOL ImportUser(LPCTSTR ini, int nUser) {
  CResStr USRKEY(_T("User%d"), nUser);
  CResStr WLKEY(_T("WhiteList%d"), nUser);
  CResStr WLFKEY(_T("WhiteListFlags%d"), nUser);
  if (GetPrivateProfileInt(USRKEY, _T("IsLocalUser"), -1, ini) == -1)
    return FALSE;
  ImportUser(ini, USRKEY, WLKEY, WLFKEY);
  return TRUE;
}

void ExportSettings(LPTSTR ini) {
  PathUnquoteSpaces(ini);
  DeleteFile(ini);
  CResStr SuRunKey(_T("SuRun"));
  WritePrivateProfileString(SuRunKey, _T("Version"), GetVersionString(), ini);
  // EXPORTINT("SuRun",UseSuRunGrp,ini);

  EXPORTINT(SuRunKey, BlurDesk, ini);
  EXPORTINT(SuRunKey, FadeDesk, ini);
  EXPORTINT(SuRunKey, SavePW, ini);
  EXPORTINT(SuRunKey, UseCancelTimeOut, ini);
  EXPORTINT(SuRunKey, CancelTimeOut, ini);
  EXPORTINT(SuRunKey, ShowCancelTimeOut, ini);

  EXPORTINT(SuRunKey, PwTimeOut, ini);
  EXPORTINT(SuRunKey, AdminNoPassWarn, ini);

  EXPORTINT(SuRunKey, CtrlAsAdmin, ini);
  EXPORTINT(SuRunKey, CmdAsAdmin, ini);
  EXPORTINT(SuRunKey, ExpAsAdmin, ini);
  EXPORTINT(SuRunKey, RestartAsAdmin, ini);
  EXPORTINT(SuRunKey, StartAsAdmin, ini);

  EXPORTINT(SuRunKey, HideExpertSettings, ini);

  EXPORTINT(SuRunKey, UseIShExHook, ini);
  EXPORTINT(SuRunKey, UseIATHook, ini);
  //  EXPORTINT(SuRunKey,UseSVCHook,ini);
  EXPORTINT(SuRunKey, TestReqAdmin, ini);
  EXPORTINT(SuRunKey, ShowAutoRuns, ini);
  EXPORTINT(SuRunKey, TrayTimeOut, ini);

  //  WritePrivateProfileInt(SuRunKey,_T("ReplaceRunAs"),
  //    IsDlgButtonChecked(g_SD->hTabCtrl[3],IDC_DORUNAS),ini);
  //  if(GetUseSuRunGrp)
  //  {
  //    EXPORTINTu(SuRunKey,SetTime,SURUNNERSGROUP,ini);
  //    EXPORTINT(SuRunKey,SetEnergy,ini);
  //  }
  //  EXPORTINT(SuRunKey,WinUpd4All,ini);
  //  EXPORTINT(SuRunKey,WinUpdBoot,ini);
  //  EXPORTINT(SuRunKey,OwnerAdminGrp,ini);

  EXPORTINT(SuRunKey, ShowTrayAdmin, ini);
  EXPORTINT(SuRunKey, UseWinLogonDesk, ini);

  EXPORTINT(SuRunKey, NoConvAdmin, ini);
  EXPORTINT(SuRunKey, NoConvUser, ini);
  EXPORTINT(SuRunKey, DefHideSuRun, ini);
  HKEY Key;
  if (RegOpenKeyEx(HKLM, HKLSTKEY, 0, KSAM(KEY_READ), &Key) == ERROR_SUCCESS) {
    TCHAR cmd[4096];
    _tcscpy(cmd, _T("\"")); // Quote strings!
    DWORD ccMax = countof(cmd) - 2;
    for (int i = 0;
         (RegEnumValue(Key, i, &cmd[1], &ccMax, 0, 0, 0, 0) == ERROR_SUCCESS);
         i++) {
      ccMax = countof(cmd) - 2;
      _tcscat(cmd, _T("\"")); // Quote strings!
      EXPORTSTR(_T("BlackList"), i, cmd, ini);
      _tcscpy(cmd, _T("\"")); // Quote strings!
    }
    RegCloseKey(Key);
  }
  for (int u = 0; u < g_SD->Users.GetCount(); u++)
    ExportUser(ini, g_SD->Users.GetUserName(u), u);
}

void ExportSettings(LPTSTR ini, DWORD SessionID, LPCTSTR User) {
  if (g_SD)
    return;
  SETUPDATA sd(SessionID, User);
  g_SD = &sd;
  ExportSettings(ini);
  g_SD = 0;
}

static BOOL GetINIFile(HWND hwnd, LPTSTR FileName) {
  CImpersonateSessionUser ilu(g_SD->SessID);
  OPENFILENAME ofn = {0};
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = TEXT("*.*\0*.*\0\0");
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = FileName;
  ofn.nMaxFile = 4096;
  ofn.lpstrTitle = 0;
  ofn.Flags = OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN | OFN_OVERWRITEPROMPT;
  ofn.FlagsEx = OFN_EX_NOPLACESBAR;
  BOOL bRet = GetSaveFileName(&ofn);
  return bRet;
}

void ImportSettings(LPCTSTR ini, bool bSuRunSettings, bool bBlackList,
                    bool bUser) {
  if (bSuRunSettings) {
    CResStr SuRunKey(_T("SuRun"));
    // IMPORTINT(SuRunKey,UseSuRunGrp,ini);

    IMPORTINT(SuRunKey, BlurDesk, ini);
    IMPORTINT(SuRunKey, FadeDesk, ini);

    IMPORTINT(SuRunKey, UseCancelTimeOut, ini);
    IMPORTINT(SuRunKey, CancelTimeOut, ini);
    IMPORTINT(SuRunKey, ShowCancelTimeOut, ini);

    IMPORTINT(SuRunKey, SavePW, ini);

    IMPORTINT(SuRunKey, PwTimeOut, ini);
    IMPORTINT(SuRunKey, AdminNoPassWarn, ini);

    IMPORTINT(SuRunKey, CtrlAsAdmin, ini);
    IMPORTINT(SuRunKey, CmdAsAdmin, ini);
    IMPORTINT(SuRunKey, ExpAsAdmin, ini);
    IMPORTINT(SuRunKey, RestartAsAdmin, ini);
    IMPORTINT(SuRunKey, StartAsAdmin, ini);

    IMPORTINT(SuRunKey, HideExpertSettings, ini);

    IMPORTINT(SuRunKey, UseIShExHook, ini);
    IMPORTINT(SuRunKey, UseIATHook, ini);
    //    IMPORTINT(SuRunKey,UseSVCHook,ini);
    IMPORTINT(SuRunKey, TestReqAdmin, ini);
    IMPORTINT(SuRunKey, ShowAutoRuns, ini);
    IMPORTINT(SuRunKey, TrayTimeOut, ini);

    //    switch(GetPrivateProfileInt(SuRunKey,_T("ReplaceRunAs"),-1,ini))
    //    {
    //    case 0:
    //      ReplaceSuRunWithRunAs();
    //      break;
    //    case 1:
    //      ReplaceRunAsWithSuRun();
    //      break;
    //    }
    //    IMPORTINT(SuRunKey,OwnerAdminGrp,ini);
    //    IMPORTINT(SuRunKey,WinUpd4All,ini);
    //    IMPORTINT(SuRunKey,WinUpdBoot,ini);
    //    if(GetUseSuRunGrp)
    //    {
    //      IMPORTINT(SuRunKey,SetEnergy,ini);
    //      IMPORTINTu(SuRunKey,SetTime,SURUNNERSGROUP,ini);
    //    }

    IMPORTINT(SuRunKey, ShowTrayAdmin, ini);
    IMPORTINT(SuRunKey, UseWinLogonDesk, ini);

    IMPORTINT(SuRunKey, NoConvAdmin, ini);
    IMPORTINT(SuRunKey, NoConvUser, ini);
    IMPORTINT(SuRunKey, DefHideSuRun, ini);
  }
  if (bBlackList) {
    DelRegKey(HKLM, HKLSTKEY);
    for (int i = 0;; i++) {
      TCHAR cmd[4096];
      IMPORTSTR(_T("BlackList"), i, cmd, ini);
      if (cmd[0])
        AddToBlackList(cmd);
      else
        break;
    }
  }
  if (bUser)
    for (int u = 0; ImportUser(ini, u); u++)
      ;
}

bool ImportSettings(LPTSTR ini) {
  PathUnquoteSpaces(ini);
  if (!PathFileExists(ini))
    return FALSE;
  ImportSettings(ini, true, true, true);
  return TRUE;
}

static BOOL OpenINIFile(HWND hwnd, LPTSTR FileName) {
  CImpersonateSessionUser ilu(g_SD->SessID);
  OPENFILENAME ofn = {0};
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFilter = TEXT("*.*\0*.*\0\0");
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = FileName;
  ofn.nMaxFile = 4096;
  ofn.lpstrTitle = 0;
  ofn.Flags = OFN_ENABLESIZING | OFN_FORCESHOWHIDDEN;
  ofn.FlagsEx = OFN_EX_NOPLACESBAR;
  BOOL bRet = GetOpenFileName(&ofn);
  return bRet;
}

INT_PTR CALLBACK ImportDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (msg) {
  case WM_INITDIALOG: {
    BringToPrimaryMonitor(hwnd);
    CheckDlgButton(hwnd, IDC_IMPSURUNSETTINGS, 1);
    CheckDlgButton(hwnd, IDC_IMPBLACKLIST, 1);
    CheckDlgButton(hwnd, IDC_IMPUSRSETTINGS, 1);
    PostMessage(hwnd, WM_COMMAND, IDC_SELFILE,
                (LPARAM)GetDlgItem(hwnd, IDC_SELFILE));
  }
    return TRUE;
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_COMMAND:
    switch (wParam) {
    case MAKELPARAM(IDC_SELFILE, BN_CLICKED): {
      TCHAR f[4096] = {0};
      GetDlgItemText(hwnd, IDC_FILENAME, f, 4096);
      if (OpenINIFile(hwnd, f)) {
        SetDlgItemText(hwnd, IDC_FILENAME, f);
        TCHAR cmd[MAX_PATH];
        bool bSRSet = GetPrivateProfileInt(L"SuRun", L"BlurDesk", -1, f) != -1;
        bool bBlLst = GetPrivateProfileString(L"BlackList", L"0", L"", cmd,
                                              MAX_PATH, f) != 0;
        bool bUsrSet =
            GetPrivateProfileInt(L"User0", L"NoRunSetup", -1, f) != -1;
        CheckDlgButton(hwnd, IDC_IMPSURUNSETTINGS, bSRSet);
        CheckDlgButton(hwnd, IDC_IMPBLACKLIST, bBlLst);
        CheckDlgButton(hwnd, IDC_IMPUSRSETTINGS, bUsrSet);
        EnableWindow(GetDlgItem(hwnd, IDC_IMPSURUNSETTINGS), bSRSet);
        EnableWindow(GetDlgItem(hwnd, IDC_IMPBLACKLIST), bBlLst);
        EnableWindow(GetDlgItem(hwnd, IDC_IMPUSRSETTINGS), bUsrSet);
      } else if (f[0] == 0)
        EndDialog(hwnd, IDCANCEL);
    } break;
    case MAKELPARAM(IDCANCEL, BN_CLICKED):
      EndDialog(hwnd, IDCANCEL);
      return TRUE;
    case MAKELPARAM(IDOK, BN_CLICKED): {
      TCHAR f[4096] = {0};
      TCHAR cmd[MAX_PATH];
      GetDlgItemText(hwnd, IDC_FILENAME, f, 4096);
      if (PathFileExists(f)) {
        bool bSRSet = GetPrivateProfileInt(L"SuRun", L"BlurDesk", -1, f) != -1;
        bool bBlLst = GetPrivateProfileString(L"BlackList", L"0", L"", cmd,
                                              MAX_PATH, f) != 0;
        bool bUsrSet =
            GetPrivateProfileInt(L"User0", L"IsLocalUser", -1, f) != -1;
        if (bSRSet || bBlLst || bUsrSet) {
          ImportSettings(
              f,
              bSRSet && (IsDlgButtonChecked(hwnd, IDC_IMPSURUNSETTINGS) != 0),
              bBlLst && (IsDlgButtonChecked(hwnd, IDC_IMPBLACKLIST) != 0),
              bUsrSet && (IsDlgButtonChecked(hwnd, IDC_IMPUSRSETTINGS) != 0));
          EndDialog(hwnd, IDOK);
        }
      }
    }
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL ImportSettings(HWND hwnd) {
  return DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_IMPORTSETTINGS),
                   hwnd, ImportDlgProc) == IDOK;
}

//////////////////////////////////////////////////////////////////////////////
//
// SetRecommendedSettings()
//
//////////////////////////////////////////////////////////////////////////////
// void SetUseSuRuners(BOOL bUse)
// {
//   if (GetUseSuRunGrp==bUse)
//     return;
//   TCHAR u[MAX_PATH];
//   _tcscpy(u,(g_SD->CurUser>=0)?g_SD->Users.GetUserName(g_SD->CurUser):g_SD->OrgUser);
//   //SetUseSuRunGrp((DWORD)bUse);
//   if (bUse)
//     CreateSuRunnersGroup();
//   else
//     DeleteSuRunnersGroup();
//   DelAllUsrSettings;
//   g_SD->Users.SetSurunnersUsers(g_SD->OrgUser,g_SD->SessID,false);
//   UpdateUserList(g_SD->hTabCtrl[1],u);
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[1],IDC_ADDUSER),bUse);
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[1],IDC_DELUSER),bUse);
//
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[3],IDC_NOCONVADMIN),bUse);
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[3],IDC_NOCONVUSER),bUse);
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[3],IDC_HIDESURUN),bUse);
//
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[3],IDC_ALLOWTIME),bUse);
//   EnableWindow(GetDlgItem(g_SD->hTabCtrl[3],IDC_SETENERGY),bUse);
// }

void SetRecommendedSettings() {
  HWND h = g_SD->hTabCtrl[0];
  CheckDlgButton(h, IDC_BLURDESKTOP, 1);
  CheckDlgButton(h, IDC_FADEDESKTOP, 1);
  CheckDlgButton(h, IDC_USE_C_TO, 1);
  SetDlgItemInt(h, IDC_CANCEL_TO, 40, 0);
  CheckDlgButton(h, IDC_SHOW_C_TO, 0);
  EnableWindow(GetDlgItem(h, IDC_CANCEL_TO), 1);
  EnableWindow(GetDlgItem(h, IDC_SHOW_C_TO), 1);
  CheckDlgButton(h, IDC_ASKPW, 0);
  SetDlgItemInt(h, IDC_ASKTIMEOUT, 0, 0);
  ComboBox_SetCurSel(GetDlgItem(h, IDC_WARNADMIN), APW_NR_SR_ADMIN);
  CheckDlgButton(h, IDC_CTRLASADMIN, 1 /*IsWin7pp==0*/);
  CheckDlgButton(h, IDC_CMDASADMIN, 0);
  CheckDlgButton(h, IDC_EXPASADMIN, 1 /*IsWin7pp==0*/);
  CheckDlgButton(h, IDC_RESTARTADMIN, 1);
  CheckDlgButton(h, IDC_STARTADMIN, 0);
  EnableWindow(GetDlgItem(h, IDC_FADEDESKTOP), !IsWin2k);
  EnableWindow(GetDlgItem(h, IDC_ASKTIMEOUT), 0);

  h = g_SD->hTabCtrl[1];
  if (g_SD->Users.GetCount()) {
    EnableWindow(GetDlgItem(h, IDC_RUNSETUP), 1);
    EnableWindow(GetDlgItem(h, IDC_REQPW4SETUP), 1);
    EnableWindow(GetDlgItem(h, IDC_RESTRICTED), 1);
    EnableWindow(GetDlgItem(h, IDC_HW_ADMIN), GetUseSVCHook);
    EnableWindow(GetDlgItem(h, IDC_HIDESURUN), 1);
    EnableWindow(GetDlgItem(h, IDC_TRAYSHOWADMIN), 1);
    EnableWindow(GetDlgItem(h, IDC_TRAYBALLOON), !IsWin2k);
    EnableWindow(GetDlgItem(h, IDC_STORE_PW), 1);
    CheckDlgButton(h, IDC_STORE_PW, 0);
    CheckDlgButton(h, IDC_RUNSETUP, 1);
    CheckDlgButton(h, IDC_REQPW4SETUP, 0);
    CheckDlgButton(h, IDC_RESTRICTED, 0);

    CheckDlgButton(h, IDC_HIDESURUN, 0);
    CheckDlgButton(h, IDC_TRAYSHOWADMIN, 1);
    CheckDlgButton(h, IDC_TRAYBALLOON, !IsWin2k);
    CheckDlgButton(h, IDC_STORE_PW, 0);
    // CheckDlgButton(h,IDC_NOUSESURUNNERS,0);
    // SetUseSuRuners(TRUE);
    int User = g_SD->CurUser;
    for (g_SD->CurUser = 0; g_SD->CurUser < g_SD->Users.GetCount();
         g_SD->CurUser++) {
      CheckDlgButton(h, IDC_HW_ADMIN,
                     GetUseSVCHook && GetInstallDevs(g_SD->Users.GetUserName(
                                          g_SD->CurUser)));
      SaveUserFlags();
    }
    g_SD->CurUser = User;
  }

  h = g_SD->hTabCtrl[2];
  CheckDlgButton(h, IDC_SHEXHOOK, 1);
  CheckDlgButton(h, IDC_IATHOOK, 1);
  CheckDlgButton(h, IDC_REQADMIN, 1);
  CheckDlgButton(h, IDC_SHOWTRAY, 1);
  SetDlgItemInt(h, IDC_TRAY_TO, 20, 0);
  EnableWindow(GetDlgItem(h, IDC_REQADMIN), 1);
  EnableWindow(GetDlgItem(h, IDC_BLACKLIST), 1);
  EnableWindow(GetDlgItem(h, IDC_SHOWTRAY), 1);
  UpdateWhiteListFlags(GetDlgItem(g_SD->hTabCtrl[1], IDC_WHITELIST));
  h = g_SD->hTabCtrl[3];
  //  CheckDlgButton(h,IDC_DORUNAS,0);
  //  CheckDlgButton(h,IDC_ALLOWTIME,0);
  //  CheckDlgButton(h,IDC_SETENERGY,1);
  //  CheckDlgButton(h,IDC_WINUPD4ALL,1);
  //  CheckDlgButton(h,IDC_WINUPDBOOT,1);
  //  CheckDlgButton(h,IDC_OWNERGROUP,1);
  ComboBox_SetCurSel(GetDlgItem(h, IDC_TRAYSHOWADMIN), TSA_ADMIN);
  CheckDlgButton(h, IDC_TRAYBALLOON, !IsWin2k);
  EnableWindow(GetDlgItem(h, IDC_TRAYBALLOON), !IsWin2k);
  CheckDlgButton(h, IDC_NOCONVADMIN, 0);
  CheckDlgButton(h, IDC_NOCONVUSER, 0);
  CheckDlgButton(h, IDC_HIDESURUN, 0);
}

void ShowExpertSettings(HWND hwnd, bool bShow) {
  HWND hTab = GetDlgItem(hwnd, IDC_SETUP_TAB);
  int nSel = TabCtrl_GetCurSel(hTab);
  TabCtrl_DeleteItem(hTab, 3);
  TabCtrl_DeleteItem(hTab, 2);
  if (!bShow) {
    // ShowWindow(GetDlgItem(g_SD->hTabCtrl[1],IDC_NOUSESURUNNERS),SW_HIDE);
    if (nSel > 1) {
      ShowWindow(g_SD->hTabCtrl[nSel], FALSE);
      ShowWindow(g_SD->hTabCtrl[1], TRUE);
      TabCtrl_SetCurSel(hTab, 1);
    }
  } else {
    // ShowWindow(GetDlgItem(g_SD->hTabCtrl[1],IDC_NOUSESURUNNERS),SW_SHOW);
    TCITEM tie3 = {TCIF_TEXT, 0, 0, CResStr(IDS_SETUP3), 0, 0, 0};
    TabCtrl_InsertItem(hTab, 2, &tie3);
    TCITEM tie4 = {TCIF_TEXT, 0, 0, CResStr(IDS_SETUP4), 0, 0, 0};
    TabCtrl_InsertItem(hTab, 3, &tie4);
  }
  RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

//////////////////////////////////////////////////////////////////////////////
//
// Dialog Proc for first Tab-Control
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SetupDlg1Proc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (msg) {
  case WM_INITDIALOG: {
    CheckDlgButton(hwnd, IDC_BLURDESKTOP, GetBlurDesk);
    CheckDlgButton(hwnd, IDC_FADEDESKTOP, GetFadeDesk);
    EnableWindow(GetDlgItem(hwnd, IDC_FADEDESKTOP), (!IsWin2k) && GetBlurDesk);

    CheckDlgButton(hwnd, IDC_USE_C_TO, GetUseCancelTimeOut);
    SendDlgItemMessage(hwnd, IDC_CANCEL_TO, EM_LIMITTEXT, 2, 0);
    SetDlgItemInt(hwnd, IDC_CANCEL_TO, GetCancelTimeOut, 0);
    CheckDlgButton(hwnd, IDC_SHOW_C_TO, GetShowCancelTimeOut);
    EnableWindow(GetDlgItem(hwnd, IDC_CANCEL_TO),
                 IsDlgButtonChecked(hwnd, IDC_USE_C_TO));
    EnableWindow(GetDlgItem(hwnd, IDC_SHOW_C_TO),
                 IsDlgButtonChecked(hwnd, IDC_USE_C_TO));

    SendDlgItemMessage(hwnd, IDC_ASKTIMEOUT, EM_LIMITTEXT, 2, 0);
    SetDlgItemInt(hwnd, IDC_ASKTIMEOUT, GetPwTimeOut, 0);
    BOOL bAsk = (!GetSavePW) || (GetPwTimeOut != 0);
    CheckDlgButton(hwnd, IDC_ASKPW, bAsk);
    EnableWindow(GetDlgItem(hwnd, IDC_ASKTIMEOUT), bAsk);

    HWND cb = GetDlgItem(hwnd, IDC_WARNADMIN);
    for (int i = 0; i < 5; i++)
      ComboBox_InsertString(cb, i, CResStr(IDS_WARNADMIN + i));
    ComboBox_SetCurSel(cb, GetAdminNoPassWarn);

    CheckDlgButton(hwnd, IDC_CTRLASADMIN, GetCtrlAsAdmin);
    CheckDlgButton(hwnd, IDC_CMDASADMIN, GetCmdAsAdmin);
    CheckDlgButton(hwnd, IDC_EXPASADMIN, GetExpAsAdmin);

    CheckDlgButton(hwnd, IDC_RESTARTADMIN, GetRestartAsAdmin);
    CheckDlgButton(hwnd, IDC_STARTADMIN, GetStartAsAdmin);

    CheckDlgButton(hwnd, IDC_NOEXPERT, !GetHideExpertSettings);
    return TRUE;
  } // WM_INITDIALOG
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(IDC_BLURDESKTOP, BN_CLICKED):
      EnableWindow(GetDlgItem(hwnd, IDC_FADEDESKTOP),
                   (!IsWin2k) && IsDlgButtonChecked(hwnd, IDC_BLURDESKTOP));
      return TRUE;
    case MAKELPARAM(IDC_USE_C_TO, BN_CLICKED):
      EnableWindow(GetDlgItem(hwnd, IDC_CANCEL_TO),
                   IsDlgButtonChecked(hwnd, IDC_USE_C_TO));
      EnableWindow(GetDlgItem(hwnd, IDC_SHOW_C_TO),
                   IsDlgButtonChecked(hwnd, IDC_USE_C_TO));
      return TRUE;
    case MAKELPARAM(IDC_ASKPW, BN_CLICKED):
      EnableWindow(GetDlgItem(hwnd, IDC_ASKTIMEOUT),
                   IsDlgButtonChecked(hwnd, IDC_ASKPW));
      return TRUE;
    case MAKELPARAM(IDC_NOEXPERT, BN_CLICKED):
      ShowExpertSettings(GetParent(hwnd),
                         IsDlgButtonChecked(hwnd, IDC_NOEXPERT) != 0);
      break;
    case MAKELPARAM(IDC_SIMPLESETUP, BN_CLICKED):
      if (SafeMsgBox(hwnd, CBigResStr(IDS_SIMPLESETUP), CResStr(IDS_APPNAME),
                     MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
        SetRecommendedSettings();
      }
      break;
    case MAKELPARAM(ID_APPLY, BN_CLICKED):
      goto ApplyChanges;
    case MAKELPARAM(IDC_IMPORT, BN_CLICKED):
      if (ImportSettings(hwnd) == IDOK) {
        g_SD->DlgExitCode = -2;
        PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
      }
      return TRUE;
    case MAKELPARAM(IDC_EXPORT, BN_CLICKED): {
      TCHAR f[4096] = {0};
      if (GetINIFile(hwnd, f)) {
        DeleteFile(f);
        ExportSettings(f);
      }
    }
      return TRUE;
    } // switch (wParam)
    break;
  } // WM_COMMAND
  case WM_DESTROY:
    if (g_SD->DlgExitCode == IDOK) // User pressed OK, save settings
    {
    ApplyChanges:
      SetBlurDesk(IsDlgButtonChecked(hwnd, IDC_BLURDESKTOP));
      SetFadeDesk(IsDlgButtonChecked(hwnd, IDC_FADEDESKTOP));

      SetUseCancelTimeOut(IsDlgButtonChecked(hwnd, IDC_USE_C_TO));
      SetCancelTimeOut(GetDlgItemInt(hwnd, IDC_CANCEL_TO, 0, 0));
      SetShowCancelTimeOut(IsDlgButtonChecked(hwnd, IDC_SHOW_C_TO));

      if (IsDlgButtonChecked(hwnd, IDC_ASKPW)) {
        SetPwTimeOut(GetDlgItemInt(hwnd, IDC_ASKTIMEOUT, 0, 0));
      } else
        SetPwTimeOut(0);
      DWORD bSave =
          (!IsDlgButtonChecked(hwnd, IDC_ASKPW)) || (GetPwTimeOut != 0);
      SetSavePW(bSave);
      SetAdminNoPassWarn(ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_WARNADMIN)));

      SetCtrlAsAdmin(IsDlgButtonChecked(hwnd, IDC_CTRLASADMIN));
      SetCmdAsAdmin(IsDlgButtonChecked(hwnd, IDC_CMDASADMIN));
      SetExpAsAdmin(IsDlgButtonChecked(hwnd, IDC_EXPASADMIN));

      SetRestartAsAdmin(IsDlgButtonChecked(hwnd, IDC_RESTARTADMIN));
      SetStartAsAdmin(IsDlgButtonChecked(hwnd, IDC_STARTADMIN));

      SetHideExpertSettings(IsDlgButtonChecked(hwnd, IDC_NOEXPERT) == 0);
      return TRUE;
    } // WM_DESTROY
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Dialog Proc for second Tab-Control
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SetupDlg2Proc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  switch (msg) {
  case WM_INITDIALOG: {
    // Program list icons:
    HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
    SetWindowLong(hWL, GWL_STYLE,
                  GetWindowLong(hWL, GWL_STYLE) | LVS_SHAREIMAGELISTS);
    SendMessage(hWL, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
    ListView_SetImageList(hWL, g_SD->ImgList, LVSIL_SMALL);
    for (int i = 0; i < 3; i++) {
      LVCOLUMN col = {LVCF_WIDTH, 0, (i == 0) ? 26 : 22, 0, 0, 0, 0, 0};
      ListView_InsertColumn(hWL, i, &col);
    }
    // CheckDlgButton(hwnd,IDC_NOUSESURUNNERS,GetUseSuRunGrp==0);
    // ShowWindow(GetDlgItem(hwnd,IDC_NOUSESURUNNERS),SW_HIDE);
    if (!GetUseSuRunGrp) {
      EnableWindow(GetDlgItem(hwnd, IDC_ADDUSER), false);
      EnableWindow(GetDlgItem(hwnd, IDC_DELUSER), false);
    }
    // UserList
    UpdateUserList(hwnd, g_SD->OrgUser);
    g_SD->Setup2Anchor.Init(hwnd);
    g_SD->Setup2Anchor.Add(IDC_USER, ANCHOR_TOPLEFT | ANCHOR_RIGHT);
    g_SD->Setup2Anchor.Add(IDC_ADDUSER, ANCHOR_TOPRIGHT);
    g_SD->Setup2Anchor.Add(IDC_DELUSER, ANCHOR_TOPRIGHT);
    g_SD->Setup2Anchor.Add(IDS_GRPDESC, ANCHOR_ALL);
    g_SD->Setup2Anchor.Add(IDC_WHITELIST, ANCHOR_ALL);
    // g_SD->Setup2Anchor.Add(IDC_NOUSESURUNNERS,ANCHOR_BOTTOMLEFT);
    g_SD->Setup2Anchor.Add(IDC_IMEXSTATIC, ANCHOR_BOTTOMLEFT);
    g_SD->Setup2Anchor.Add(IDC_IMPORT, ANCHOR_BOTTOMLEFT);
    g_SD->Setup2Anchor.Add(IDC_EXPORT, ANCHOR_BOTTOMLEFT);
    g_SD->Setup2Anchor.Add(IDC_ADDAPP, ANCHOR_BOTTOMRIGHT);
    g_SD->Setup2Anchor.Add(IDC_EDITAPP, ANCHOR_BOTTOMRIGHT);
    g_SD->Setup2Anchor.Add(IDC_DELETE, ANCHOR_BOTTOMRIGHT);
    EnableWindow(GetDlgItem(hwnd, IDC_HW_ADMIN), GetUseSVCHook);
    return TRUE;
  } // WM_INITDIALOG
  case WM_SIZE: {
    g_SD->Setup2Anchor.OnSize(false);
    return TRUE;
  }
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_PAINT:
    // The List Control is (to for some to me unknow reason) NOT displayed if
    // a user app switches to the user desktop and WatchDog switches back.
    RedrawWindow(GetDlgItem(hwnd, IDC_WHITELIST), 0, 0,
                 RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
    break;
  case WM_TIMER: {
    if (wParam == 1)
      UpdateUser(hwnd);
    return TRUE;
  } // WM_TIMER
  case WM_COMMAND: {
    switch (wParam) {
    // User Combobox
    case MAKEWPARAM(IDC_USER, CBN_DROPDOWN):
      SetTimer(hwnd, 1, 250, 0);
      return TRUE;
    case MAKEWPARAM(IDC_USER, CBN_CLOSEUP):
      KillTimer(hwnd, 1);
      return TRUE;
    case MAKEWPARAM(IDC_USER, CBN_SELCHANGE):
    case MAKEWPARAM(IDC_USER, CBN_EDITCHANGE):
      UpdateUser(hwnd);
      return TRUE;
    case MAKELPARAM(IDC_IMPORT, BN_CLICKED): {
      TCHAR f[4096] = {0};
      LPCTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
      if (OpenINIFile(hwnd, f) &&
          ImportUserWhiteList(f, u, _T("WhiteList"), _T("WhiteListFlags")))
        UpdateUserList(hwnd, u);
    }
      return TRUE;
    case MAKELPARAM(IDC_EXPORT, BN_CLICKED): {
      TCHAR f[4096] = {0};
      if (GetINIFile(hwnd, f)) {
        DeleteFile(f);
        ExportUserWhiteList(f, g_SD->Users.GetUserName(g_SD->CurUser),
                            _T("WhiteList"), _T("WhiteListFlags"));
      }
    }
      return TRUE;
      //      case MAKELPARAM(IDC_NOUSESURUNNERS,BN_CLICKED):
      //        SetUseSuRuners(IsDlgButtonChecked(hwnd,IDC_NOUSESURUNNERS)==0);
      //        return TRUE;
      // Edit Button
    case MAKELPARAM(IDC_EDITAPP, BN_CLICKED): {
    EditApp:
      HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
      int nSel = ListView_GetSelectionMark(hWL);
      int cSel = ListView_GetSelectedCount(hWL);
      if ((nSel >= 0) && (cSel == 1)) {
        TCHAR cmd[4096];
        TCHAR CMD[4096];
        ListView_GetItemText(hWL, nSel, 2, cmd, 4096);
        _tcscpy(CMD, cmd);
        LPTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
        DWORD f = GetWhiteListFlags(u, cmd, 0);
        if (GetFileName(hwnd, f, CMD)) {
          if (RemoveFromWhiteList(u, cmd)) {
            if (AddToWhiteList(u, CMD, f)) {
              ListView_DeleteItem(hWL, nSel);
              LVITEM item = {LVIF_IMAGE,          0, 0, 0, 0, 0, 0,
                             g_SD->ImgIconIdx[0], 0, 0};
              ListView_SetItemText(hWL, ListView_InsertItem(hWL, &item), 2,
                                   CMD);
              ListView_SortItemsEx(hWL, ListSortProc, hWL);
              UpdateWhiteListFlags(hWL);
            } else
              MessageBeep(MB_ICONERROR);
          } else
            MessageBeep(MB_ICONERROR);
        }
        RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
      }
    }
      return TRUE;
    // Add User Button
    case MAKELPARAM(IDC_ADDUSER, BN_CLICKED): {
      zero(g_SD->NewUser);
      DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_SELUSER), hwnd,
                SelUserDlgProc);
      bool IsAdmin = IsInAdmins(g_SD->NewUser, g_SD->SessID) != 0;
      bool IsSplitAdmin = IsAdmin && UACEnabled;
      if (g_SD->NewUser[0] && (!IsInSuRunners(g_SD->NewUser, g_SD->SessID)) &&
          BecomeSuRunner(g_SD->NewUser, g_SD->SessID, IsAdmin, IsSplitAdmin,
                         FALSE, hwnd)) {
        g_SD->Users.SetSurunnersUsers(g_SD->OrgUser, g_SD->SessID, TRUE);
        UpdateUserList(hwnd, g_SD->NewUser);
        zero(g_SD->NewUser);
      }
    }
      return TRUE;
    // Delete User Button
    case MAKELPARAM(IDC_DELUSER, BN_CLICKED):
      if (GetUseSuRunGrp) {
        LPTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
        switch (SafeMsgBox(hwnd, CBigResStr(IDS_DELUSER, u),
                           CResStr(IDS_APPNAME),
                           MB_ICONASTERISK | MB_YESNOCANCEL | MB_DEFBUTTON3)) {
        case IDYES:
          AlterGroupMember(DOMAIN_ALIAS_RID_ADMINS, u, 1);
          // Fall through
        case IDNO:
          AlterGroupMember(SURUNNERSGROUP, u, 0);
          DelUsrSettings(u);
          g_SD->Users.SetSurunnersUsers(g_SD->OrgUser, g_SD->SessID, TRUE);
          UpdateUserList(hwnd, g_SD->OrgUser);
          break;
        }
      }
      return TRUE;
    // Add App Button
    case MAKELPARAM(IDC_ADDAPP, BN_CLICKED): {
      TCHAR cmd[4096] = {0};
      DWORD f = 0;
      if (GetFileName(hwnd, f, cmd)) {
        if (AddToWhiteList(g_SD->Users.GetUserName(g_SD->CurUser), cmd, f)) {
          HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
          LVITEM item = {LVIF_IMAGE,          0, 0, 0, 0, 0, 0,
                         g_SD->ImgIconIdx[0], 0, 0};
          ListView_SetItemText(hWL, ListView_InsertItem(hWL, &item), 2, cmd);
          ListView_SortItemsEx(hWL, ListSortProc, hWL);
          UpdateWhiteListFlags(hWL);
        } else
          MessageBeep(MB_ICONERROR);
      }
    }
      return TRUE;
    case MAKELPARAM(IDC_RESTRICTED, BN_CLICKED):
      UpdateWhiteListFlags(GetDlgItem(hwnd, IDC_WHITELIST));
      return TRUE;
    // Delete App Button
    case MAKELPARAM(IDC_DELETE, BN_CLICKED): {
    DelApp:
      HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
      int cSel = ListView_GetSelectedCount(hWL);
      if (cSel > 0)
        for (int n = 0; n < ListView_GetItemCount(hWL);) {
          if (ListView_GetItemState(hWL, n, LVIS_SELECTED)) {
            TCHAR cmd[4096];
            ListView_GetItemText(hWL, n, 2, cmd, 4096);
            if (RemoveFromWhiteList(g_SD->Users.GetUserName(g_SD->CurUser),
                                    cmd)) {
              ListView_DeleteItem(hWL, n);
            } else
              MessageBeep(MB_ICONERROR);
          } else
            n++;
        }
      int nSel = ListView_GetSelectionMark(hWL);
      if (nSel >= 0)
        ListView_SetItemState(hWL, nSel, LVIS_SELECTED, 0x0F);
      EnableWindow(GetDlgItem(hwnd, IDC_DELETE), nSel >= 0);
      EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), nSel >= 0);
    }
      return TRUE;
    case MAKELPARAM(ID_APPLY, BN_CLICKED):
      goto ApplyChanges;
    case MAKELPARAM(IDC_RUNSETUP, BN_CLICKED):
      EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP),
                   IsDlgButtonChecked(hwnd, IDC_RUNSETUP));
      return TRUE;
    case MAKELPARAM(IDC_HIDESURUN, BN_CLICKED):
      if (IsDlgButtonChecked(hwnd, IDC_HIDESURUN)) {
        if (_tcscmp(g_SD->OrgUser, g_SD->Users.GetUserName(g_SD->CurUser)) ==
            0) {
          if (SafeMsgBox(hwnd, CBigResStr(IDS_HIDESELF), CResStr(IDS_APPNAME),
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDNO) {
            CheckDlgButton(hwnd, IDC_HIDESURUN, 0);
            return TRUE;
          }
        }
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_REQPW4SETUP), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_RUNSETUP), 0);
        EnableWindow(GetDlgItem(hwnd, IDC_STORE_PW), 0);
        CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_TRAYSHOWADMIN, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_RUNSETUP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_REQPW4SETUP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_RESTRICTED, BST_CHECKED);
        CheckDlgButton(hwnd, IDC_HW_ADMIN,
                       GetUseSVCHook && GetInstallDevs(g_SD->Users.GetUserName(
                                            g_SD->CurUser)));
        CheckDlgButton(hwnd, IDC_RUNSETUP, BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_STORE_PW, BST_UNCHECKED);
      } else {
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN), 1);
        BOOL bBal = (!IsWin2k) && (IsDlgButtonChecked(hwnd, IDC_TRAYSHOWADMIN));
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), bBal);
        EnableWindow(GetDlgItem(hwnd, IDC_RUNSETUP), 1);
        EnableWindow(GetDlgItem(hwnd, IDC_RESTRICTED), 1);
        EnableWindow(GetDlgItem(hwnd, IDC_STORE_PW), 1);
        CheckDlgButton(hwnd, IDC_HW_ADMIN,
                       GetUseSVCHook && GetInstallDevs(g_SD->Users.GetUserName(
                                            g_SD->CurUser)));
      }
      UpdateWhiteListFlags(GetDlgItem(hwnd, IDC_WHITELIST));
      return TRUE;
    case MAKELPARAM(IDC_TRAYSHOWADMIN, BN_CLICKED):
      if (!IsWin2k) {
        BOOL bTSA = IsDlgButtonChecked(hwnd, IDC_TRAYSHOWADMIN) == BST_CHECKED;
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), bTSA);
        if (!bTSA)
          CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_UNCHECKED);
      }
      return TRUE;
    } // switch (wParam)
    break;
  } // WM_COMMAND
  case WM_NOTIFY: {
    switch (wParam) {
    // Program List Notifications
    case IDC_WHITELIST:
      if (lParam)
        switch (((LPNMHDR)lParam)->code) {
        // Selection changed
        case LVN_ITEMCHANGED: {
          HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
          int cSel = ListView_GetSelectedCount(hWL);
          EnableWindow(GetDlgItem(hwnd, IDC_DELETE), cSel >= 1);
          EnableWindow(GetDlgItem(hwnd, IDC_EDITAPP), cSel == 1);
        }
          return TRUE;
        case LVN_KEYDOWN:
          switch (((LPNMLVKEYDOWN)lParam)->wVKey) {
          case VK_F2:
            goto EditApp;
          case VK_DELETE:
            goto DelApp;
          case (WORD)'A':
            if (GetKeyState(VK_CONTROL) & 0x8000) {
              HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
              if (ListView_GetItemCount(hWL)) {
                for (int n = 0; n < ListView_GetItemCount(hWL); n++)
                  ListView_SetItemState(hWL, n, LVIS_SELECTED, 0x0F);
                ListView_SetItemState(hWL, 0, LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);
              }
              return TRUE;
            }
          }
          break;
        // Mouse Click: Toggle Flags
        case NM_DBLCLK:
          if (((LPNMITEMACTIVATE)lParam)->iSubItem > 1)
            goto EditApp;
        case NM_CLICK: {
          LPNMITEMACTIVATE p = (LPNMITEMACTIVATE)lParam;
          int Flag = 0;
          switch (p->iSubItem) {
          case 0:
            Flag = FLAG_DONTASK;
            break;
          case 1:
            Flag = FLAG_SHELLEXEC;
            break;
          }
          if (Flag) {
            TCHAR cmd[4096];
            HWND hWL = GetDlgItem(hwnd, IDC_WHITELIST);
            ListView_GetItemText(hWL, p->iItem, 2, cmd, 4096);
            if (ToggleWhiteListFlag(g_SD->Users.GetUserName(g_SD->CurUser), cmd,
                                    Flag))
              UpdateWhiteListFlags(hWL);
            else
              MessageBeep(MB_ICONERROR);
          }
        }
          return TRUE;
        } // switch (switch(((LPNMHDR)lParam)->code)
    } // switch (wParam)
    break;
  } // WM_NOTIFY
  case WM_DESTROY:
    if (g_SD->HelpWnd)
      DestroyWindow(g_SD->HelpWnd);
    g_SD->HelpWnd = 0;
    if (g_SD->DlgExitCode != -2) {
    ApplyChanges:
      SaveUserFlags();
      return TRUE;
    } // WM_DESTROY
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Dialog Proc for third Tab-Control
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SetupDlg3Proc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (msg) {
  case WM_INITDIALOG: {
    CheckDlgButton(hwnd, IDC_SHEXHOOK, GetUseIShExHook);
    CheckDlgButton(hwnd, IDC_IATHOOK, GetUseIATHook);
    // CheckDlgButton(hwnd,IDC_SVCHOOK,GetUseSVCHook);
    // EnableWindow(GetDlgItem(g_SD->hTabCtrl[1],IDC_HW_ADMIN),GetUseSVCHook);
    CheckDlgButton(hwnd, IDC_SHOWTRAY, GetShowAutoRuns);
    CheckDlgButton(hwnd, IDC_REQADMIN, GetTestReqAdmin);
    // EnableWindow(GetDlgItem(hwnd,IDC_SVCHOOK),GetUseIATHook);
    BOOL bHook = GetUseIShExHook || GetUseIATHook;
    EnableWindow(GetDlgItem(hwnd, IDC_REQADMIN), bHook);
    EnableWindow(GetDlgItem(hwnd, IDC_BLACKLIST), bHook);
    EnableWindow(GetDlgItem(hwnd, IDC_SHOWTRAY), bHook);
    SendDlgItemMessage(hwnd, IDC_TRAY_TO, EM_LIMITTEXT, 3, 0);
    SetDlgItemInt(hwnd, IDC_TRAY_TO, GetTrayTimeOut, 0);
    return TRUE;
  } // WM_INITDIALOG
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_DESTROY:
    if (g_SD->DlgExitCode == IDOK) // User pressed OK, save settings
    {
    ApplyChanges:
      SetUseIShExHook(IsDlgButtonChecked(hwnd, IDC_SHEXHOOK));
      SetUseIATHook(IsDlgButtonChecked(hwnd, IDC_IATHOOK));
      // SetUseSVCHook(IsDlgButtonChecked(hwnd,IDC_SVCHOOK));
      SetTestReqAdmin(IsDlgButtonChecked(hwnd, IDC_REQADMIN));
      SetShowAutoRuns(IsDlgButtonChecked(hwnd, IDC_SHOWTRAY));
      EnableWindow(GetDlgItem(g_SD->hTabCtrl[1], IDC_HW_ADMIN), GetUseSVCHook);
      LPTSTR u = g_SD->Users.GetUserName(g_SD->CurUser);
      CheckDlgButton(g_SD->hTabCtrl[1], IDC_HW_ADMIN,
                     GetUseSVCHook && GetInstallDevs(u));
      SetTrayTimeOut(GetDlgItemInt(hwnd, IDC_TRAY_TO, 0, 0));
      return TRUE;
    } // WM_DESTROY
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(IDC_IATHOOK, BN_CLICKED):
    case MAKELPARAM(IDC_SVCHOOK, BN_CLICKED):
    case MAKELPARAM(IDC_SHEXHOOK, BN_CLICKED): {
      UpdateWhiteListFlags(GetDlgItem(g_SD->hTabCtrl[1], IDC_WHITELIST));
      EnableWindow(GetDlgItem(hwnd, IDC_SVCHOOK),
                   IsDlgButtonChecked(hwnd, IDC_IATHOOK));
      BOOL bHook = IsDlgButtonChecked(hwnd, IDC_SHEXHOOK) ||
                   IsDlgButtonChecked(hwnd, IDC_IATHOOK);
      EnableWindow(GetDlgItem(hwnd, IDC_REQADMIN), bHook);
      EnableWindow(GetDlgItem(hwnd, IDC_BLACKLIST), bHook);
      EnableWindow(GetDlgItem(hwnd, IDC_SHOWTRAY), bHook);
    }
      return TRUE;
    case MAKELPARAM(IDC_BLACKLIST, BN_CLICKED):
      DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_BLKLST), hwnd,
                BlkLstDlgProc);
      return TRUE;
    case MAKELPARAM(ID_APPLY, BN_CLICKED):
      goto ApplyChanges;
    }
  }
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Dialog Proc for fourth Tab-Control
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK SetupDlg4Proc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (msg) {
  case WM_INITDIALOG: {
    CheckDlgButton(hwnd, IDC_NOLOGONDESK, GetUseWinLogonDesk == 0);
    SetDlgItemInt(hwnd, IDC_START_DELAY,
                  max(0, min(600, GetRegInt(HKLM, SURUNKEY, L"StartDelay", 0))),
                  false);
    CheckDlgButton(hwnd, IDC_DORUNAS, GetHandleRunAs);
    if (GetUseSuRunGrp) {
      CheckDlgButton(hwnd, IDC_ALLOWTIME, GetSetTime(SURUNNERSGROUP));
      CheckDlgButton(hwnd, IDC_SETENERGY, GetSetEnergy);
    } else {
      EnableWindow(GetDlgItem(hwnd, IDC_ALLOWTIME), false);
      EnableWindow(GetDlgItem(hwnd, IDC_SETENERGY), false);
    }
    CheckDlgButton(hwnd, IDC_OWNERGROUP, GetOwnerAdminGrp);
    CheckDlgButton(hwnd, IDC_WINUPD4ALL, GetWinUpd4All);
    CheckDlgButton(hwnd, IDC_WINUPDBOOT, GetWinUpdBoot);

    HWND cb = GetDlgItem(hwnd, IDC_TRAYSHOWADMIN);
    DWORD tsa = GetShowTrayAdmin;
    ComboBox_InsertString(cb, 0, CResStr(IDS_WARNADMIN5)); // No users
    ComboBox_InsertString(cb, 1, CResStr(IDS_WARNADMIN));  //"All users"
    ComboBox_InsertString(cb, 2, CResStr(IDS_WARNADMIN4)); //"Administrators"
    ComboBox_SetCurSel(cb, tsa & (~(TSA_TIPS | TSA_HIDE_NORMAL)));
    CheckDlgButton(hwnd, IDC_TRAY_HIDE_NORM_ICON, (tsa & TSA_HIDE_NORMAL) != 0);
    CheckDlgButton(hwnd, IDC_TRAYBALLOON, (tsa & TSA_TIPS) != 0);
    if (IsWin2k)
      // Win2k:no balloon tips
      EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), 0);
    else
      EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), (tsa & (~TSA_TIPS)) != 0);
    CheckDlgButton(hwnd, IDC_NOCONVADMIN, GetNoConvAdmin);
    CheckDlgButton(hwnd, IDC_NOCONVUSER, GetNoConvUser);
    CheckDlgButton(hwnd, IDC_HIDESURUN, GetDefHideSuRun);

    bool bUse = GetUseSuRunGrp;
    EnableWindow(GetDlgItem(hwnd, IDC_NOCONVADMIN), bUse);
    EnableWindow(GetDlgItem(hwnd, IDC_NOCONVUSER), bUse);
    EnableWindow(GetDlgItem(hwnd, IDC_HIDESURUN), bUse);
    EnableWindow(GetDlgItem(hwnd, IDC_ALLOWTIME), bUse);
    EnableWindow(GetDlgItem(hwnd, IDC_SETENERGY), bUse);

    return TRUE;
  } // WM_INITDIALOG
  case WM_CTLCOLORSTATIC:
    SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
    SetBkMode((HDC)wParam, TRANSPARENT);
  case WM_CTLCOLORDLG:
    return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
  case WM_DESTROY:
    if (g_SD->DlgExitCode == IDOK) // User pressed OK, save settings
    {
    ApplyChanges:
      SetUseWinLogonDesk(IsDlgButtonChecked(hwnd, IDC_NOLOGONDESK) == 0);
      SetRegInt(HKLM, SURUNKEY, L"StartDelay",
                max(0, min(600, GetDlgItemInt(hwnd, IDC_START_DELAY, 0, 0))));
      switch (IsDlgButtonChecked(hwnd, IDC_DORUNAS)) {
      case BST_CHECKED:
        ReplaceRunAsWithSuRun();
        break;
      case BST_UNCHECKED:
        ReplaceSuRunWithRunAs();
        break;
      }
      if (GetUseSuRunGrp) {
        if ((GetSetTime(SURUNNERSGROUP) != 0) !=
            ((int)IsDlgButtonChecked(hwnd, IDC_ALLOWTIME)))
          SetSetTime(SURUNNERSGROUP, IsDlgButtonChecked(hwnd, IDC_ALLOWTIME));
        if (GetSetEnergy != (int)IsDlgButtonChecked(hwnd, IDC_SETENERGY))
          SetSetEnergy(IsDlgButtonChecked(hwnd, IDC_SETENERGY) != 0);
      }
      SetWinUpd4All(IsDlgButtonChecked(hwnd, IDC_WINUPD4ALL));
      SetWinUpdBoot(IsDlgButtonChecked(hwnd, IDC_WINUPDBOOT));
      SetOwnerAdminGrp(IsDlgButtonChecked(hwnd, IDC_OWNERGROUP));

      SetNoConvAdmin(IsDlgButtonChecked(hwnd, IDC_NOCONVADMIN));
      SetNoConvUser(IsDlgButtonChecked(hwnd, IDC_NOCONVUSER));
      SetDefHideSuRun(IsDlgButtonChecked(hwnd, IDC_HIDESURUN));
      DWORD tsa = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN));
      if (IsDlgButtonChecked(hwnd, IDC_TRAY_HIDE_NORM_ICON))
        tsa |= TSA_HIDE_NORMAL;
      if (IsDlgButtonChecked(hwnd, IDC_TRAYBALLOON))
        tsa |= TSA_TIPS;
      SetShowTrayAdmin(tsa);
      return TRUE;
    } // WM_DESTROY
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(IDC_TRAYSHOWADMIN, CBN_SELCHANGE):
      if (!IsWin2k) {
        BOOL bTSA = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TRAYSHOWADMIN)) > 0;
        EnableWindow(GetDlgItem(hwnd, IDC_TRAY_HIDE_NORM_ICON), bTSA);
        EnableWindow(GetDlgItem(hwnd, IDC_TRAYBALLOON), bTSA);
        if (!bTSA)
          CheckDlgButton(hwnd, IDC_TRAYBALLOON, BST_UNCHECKED);
      }
      return TRUE;
    case MAKELPARAM(ID_APPLY, BN_CLICKED):
      goto ApplyChanges;
    }
  }
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Main Setup Dialog Proc
//
//////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK MainSetupDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                                  LPARAM lParam) {
  switch (msg) {
  case WM_ACTIVATEAPP:
    if ((wParam == TRUE) && g_RunOnNewDesk) {
      HWND wd = g_RunOnNewDesk->GetDeskWnd();
      if (wd)
        SendMessage(wd, WM_MOUSEACTIVATE, 0, HTSYSMENU);
      SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
      SetForegroundWindow(hwnd);
      return TRUE;
    }
    break;
  case WM_INITDIALOG: {
    BringToPrimaryMonitor(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    SetForegroundWindow(hwnd);
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
    // Tab Control
    HWND hTab = GetDlgItem(hwnd, IDC_SETUP_TAB);
    int TabNames[nTabs] = {IDS_SETUP1, IDS_SETUP2, IDS_SETUP3, IDS_SETUP4};
    int TabIDs[nTabs] = {IDD_SETUP1, IDD_SETUP2, IDD_SETUP3, IDD_SETUP4};
    DLGPROC TabProcs[nTabs] = {SetupDlg1Proc, SetupDlg2Proc, SetupDlg3Proc,
                               SetupDlg4Proc};
    int i;
    for (i = 0; i < nTabs; i++) {
      TCITEM tie = {TCIF_TEXT, 0, 0, CResStr(TabNames[i]), 0, 0, 0};
      TabCtrl_InsertItem(hTab, i, &tie);
      g_SD->hTabCtrl[i] = CreateDialog(
          GetModuleHandle(0), MAKEINTRESOURCE(TabIDs[i]), hwnd, TabProcs[i]);
      RECT r;
      GetWindowRect(hTab, &r);
      TabCtrl_AdjustRect(hTab, FALSE, &r);
      ScreenToClient(hwnd, (POINT *)&r.left);
      ScreenToClient(hwnd, (POINT *)&r.right);
      SetWindowPos(g_SD->hTabCtrl[i], hTab, r.left, r.top, r.right - r.left,
                   r.bottom - r.top, 0);
    }
    TabCtrl_SetCurSel(hTab, 0);
    ShowWindow(g_SD->hTabCtrl[0], TRUE);
    //...
    UpdateWhiteListFlags(GetDlgItem(g_SD->hTabCtrl[1], IDC_WHITELIST));
    //...
    if (GetHideExpertSettings)
      ShowExpertSettings(hwnd, false);
    SetFocus(hTab);
    g_SD->MainSetupAnchor.Init(hwnd);
    g_SD->MainSetupAnchor.Add(IDC_SETUP_TAB, ANCHOR_ALL);
    for (i = 0; i < nTabs; i++)
      g_SD->MainSetupAnchor.Add(g_SD->hTabCtrl[i], ANCHOR_ALL);
    g_SD->MainSetupAnchor.Add(ID_APPLY, ANCHOR_BOTTOMRIGHT);
    g_SD->MainSetupAnchor.Add(IDOK, ANCHOR_BOTTOMRIGHT);
    g_SD->MainSetupAnchor.Add(IDCANCEL, ANCHOR_BOTTOMRIGHT);
    RECT r;
    GetWindowRect(hwnd, &r);
    g_SD->MinW = r.right - r.left;
    g_SD->MinH = r.bottom - r.top;
    int w = GetRegInt(HKLM, SURUNKEY, L"SetupW", g_SD->MinW);
    int h = GetRegInt(HKLM, SURUNKEY, L"SetupH", g_SD->MinH);
    int x = r.left - (w - g_SD->MinW) / 2;
    int y = r.top - (h - g_SD->MinH) / 2;
    MoveWindow(hwnd, x, y, w, h, false);
    PostMessage(hwnd, WM_SIZE, 0, 0);
    return FALSE;
  } // WM_INITDIALOG
  case WM_CLOSE: {
    if (g_SD->DlgExitCode == -2) {
      EndDialog(hwnd, -2);
      return TRUE;
    }
    break;
  }
  case WM_SIZE: {
    g_SD->MainSetupAnchor.OnSize(false);
    RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
    return TRUE;
  }
  case WM_GETMINMAXINFO: {
    MINMAXINFO *lpMMI = (MINMAXINFO *)lParam;
    lpMMI->ptMinTrackSize.x = g_SD->MinW;
    lpMMI->ptMinTrackSize.y = g_SD->MinH;
    return TRUE;
  }
  case WM_NCDESTROY: {
    RECT r;
    GetWindowRect(hwnd, &r);
    SetRegInt(HKLM, SURUNKEY, L"SetupW", r.right - r.left);
    SetRegInt(HKLM, SURUNKEY, L"SetupH", r.bottom - r.top);
    for (int i = 0; i < nTabs; i++)
      DestroyWindow(g_SD->hTabCtrl[i]);
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
    return TRUE;
  } // WM_NCDESTROY
  case WM_NOTIFY: {
    if (wParam == IDC_SETUP_TAB) {
      int nSel = TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_SETUP_TAB));
      switch (((LPNMHDR)lParam)->code) {
      case TCN_SELCHANGING:
        ShowWindow(g_SD->hTabCtrl[nSel], FALSE);
        return TRUE;
      case TCN_SELCHANGE:
        ShowWindow(g_SD->hTabCtrl[nSel], TRUE);
        return TRUE;
      }
    }
    break;
  } // WM_NOTIFY
  case WM_COMMAND: {
    switch (wParam) {
    case MAKELPARAM(ID_APPLY, BN_CLICKED):
      SendMessage(
          g_SD->hTabCtrl[TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_SETUP_TAB))],
          WM_COMMAND, wParam, lParam);
      return TRUE;
    case MAKELPARAM(IDCANCEL, BN_CLICKED):
      g_SD->DlgExitCode = IDCANCEL;
      EndDialog(hwnd, 0);
      return TRUE;
    case MAKELPARAM(IDOK, BN_CLICKED):
      g_SD->DlgExitCode = IDOK;
      EndDialog(hwnd, 1);
      return TRUE;
    } // switch (wParam)
    break;
  } // WM_COMMAND
  }
  return FALSE;
}

BOOL RunSetup(DWORD SessionID, LPCTSTR User) {
  INT_PTR bRet = -2;
  while (bRet == -2) {
    SETUPDATA sd(SessionID, User);
    g_SD = &sd;
    bRet = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_SETUP_MAIN), 0,
                     MainSetupDlgProc);
    g_SD = 0;
  }
  return bRet == IDOK;
}

#ifdef _DEBUGSETUP

#include "WinStaDesk.h"
BOOL TestSetup() {
  LoadLibrary(TEXT("Shell32.dll"));
  INITCOMMONCONTROLSEX icce = {sizeof(icce), 0xFFFF};
  TCHAR un[2 * MAX_PATH + 1];
  GetProcessUserName(GetCurrentProcessId(), un);
  InitCommonControlsEx(&icce);

  SetThreadLocale(
      MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), SORT_DEFAULT));
  if (!RunSetup(0, un))
    DBGTrace1("DialogBox failed: %s", GetLastErrorNameStatic());

  SetThreadLocale(
      MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
  if (!RunSetup(0, un))
    DBGTrace1("DialogBox failed: %s", GetLastErrorNameStatic());

  SetThreadLocale(
      MAKELCID(MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH), SORT_DEFAULT));
  if (!RunSetup(0, un))
    DBGTrace1("DialogBox failed: %s", GetLastErrorNameStatic());
  ::ExitProcess(0);
  // return TRUE;
}

#endif _DEBUGSETUP
