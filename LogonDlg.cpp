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
#include <WinCrypt.h>
#include <commctrl.h>
#include <lm.h>
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>

#include "DBGTrace.h"
#include "Helpers.h"
#include "Isadmin.h"
#include "ResStr.h"
#include "Resource.h"
#include "Setup.h"
#include "UserGroups.h"
#include "WinStaDesk.h"
#include "sspi_auth.h"



#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "comctl32.lib")

#ifndef SuRunEXT_EXPORTS
#ifdef DoDBGTrace
extern DWORD g_RunTimes[16];
extern LPCTSTR g_RunTimeNames[16];
extern DWORD g_nTimes;
#define AddTime(s)                                                             \
  {                                                                            \
    g_RunTimes[g_nTimes] = timeGetTime();                                      \
    g_RunTimeNames[g_nTimes++] = _TEXT(s);                                     \
  }
#endif //DoDBGTrace
#endif //SuRunEXT_EXPORTS
extern TOKEN_STATISTICS g_AdminTStat;

//////////////////////////////////////////////////////////////////////////////
//
//  Logon Helper
//
//////////////////////////////////////////////////////////////////////////////

HANDLE GetUserToken(DWORD SessionId, LPCTSTR User, LPCTSTR Password,
                    bool AllowEmptyPassword) {
  if ((User == 0) || (*User == 0))
    return NULL;
  TCHAR un[2 * UNLEN + 2] = {0};
  TCHAR dn[2 * UNLEN + 2] = {0};
  _tcscpy(un, User);
  PathStripPath(un);
  _tcscpy(dn, User);
  PathRemoveFileSpec(dn);
  HANDLE hToken = NULL;
  EnablePrivilege(SE_CHANGE_NOTIFY_NAME);
  EnablePrivilege(SE_TCB_NAME); // Win2k
  // Enable use of empty passwords for network logon
  BOOL bEmptyPWAllowed = FALSE;
  bool bFirstTry = FALSE;
  if (AllowEmptyPassword) {
    bFirstTry = TRUE;
    if ((Password == NULL) || (*Password == NULL)) {
      bEmptyPWAllowed = EmptyPWAllowed;
      AllowEmptyPW(TRUE);
    }
  }
  {
    CImpersonateSessionUser ilu(SessionId);
  SecondTry:
    if (!LogonUser(un, dn, Password, LOGON32_LOGON_NETWORK, 0, &hToken)) {
      if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
        hToken = SSPLogonUser(dn, un, Password);
      } // else
      // DBGTrace3("LogonUser(%s,%s) failed:
      // %s",un,dn,GetLastErrorNameStatic());
    }
    // Windows sometimes reports an error if the user's password is empty, try
    // again:
    if (bFirstTry && (hToken == NULL) &&
        ((Password == NULL) || (*Password == NULL))) {
      bFirstTry = FALSE;
      // DBGTrace2("GetUserToken(%s,%s) Second Try...",un,dn);
      goto SecondTry;
    }
  }
  //  DBGTrace4("GetUserToken(%s,%s,%s):%s",un,dn,Password,hToken?_T("SUCCEEDED."):_T("FAILED!"));
  if (AllowEmptyPassword) {
    // Reset status of "use of empty passwords for network logon"
    if ((Password == NULL) || (*Password == NULL))
      AllowEmptyPW(bEmptyPWAllowed);
  }
  return hToken;
}

BOOL PasswordOK(DWORD SessionId, LPCTSTR User, LPCTSTR Password,
                bool AllowEmptyPassword) {
  HANDLE hToken = GetUserToken(SessionId, User, Password, AllowEmptyPassword);
  BOOL bRet = hToken != 0;
  CloseHandle(hToken);
  return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  Password cache
//
//////////////////////////////////////////////////////////////////////////////

static BYTE KEYPASS[16] = {0x4f, 0xc9, 0x4d, 0x14, 0x63, 0xa9, 0x4d, 0xe2,
                           0x96, 0x47, 0x2b, 0x6a, 0xd6, 0x80, 0xd3, 0xc2};

bool LoadRunAsPassword(LPTSTR RunAsUser, LPTSTR UserName, LPTSTR Password,
                       DWORD nBytes) {
  if ((RunAsUser == 0) || (*RunAsUser == 0) || (UserName == 0) ||
      (*UserName == 0))
    return false;
  if (!GetSavePW)
    return false;
  DWORD Type = REG_BINARY;
  BYTE *ePassword;
  DWORD nB = 0;
  if (!GetRegAnyAlloc(HKLM, RAPASSKEY(RunAsUser), UserName, &Type, &ePassword,
                      &nB))
    return false;
  if (!ePassword)
    return false;
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
    return true;
  }
  return false;
}

void DeleteRunAsPassword(LPTSTR RunAsUser, LPTSTR UserName) {
  if ((RunAsUser == 0) || (*RunAsUser == 0) || (UserName == 0) ||
      (*UserName == 0))
    return;
  RegDelVal(HKLM, RAPASSKEY(RunAsUser), UserName);
}

void SaveRunAsPassword(LPTSTR RunAsUser, LPTSTR UserName, LPTSTR Password) {
  if ((RunAsUser == 0) || (*RunAsUser == 0) || (UserName == 0) ||
      (*UserName == 0))
    return;
  if (!GetSavePW)
    return;
  DATA_BLOB pw = {0};
  DATA_BLOB entropy = {sizeof(KEYPASS), KEYPASS};
  DATA_BLOB PW = {static_cast<DWORD>((DWORD)(_tcslen(Password) + 1) * sizeof(TCHAR)),
                  (BYTE *)Password};
  if (!CryptProtectData(&PW, _T("SuRunRAPW"), &entropy, 0, 0,
                        CRYPTPROTECT_UI_FORBIDDEN, &pw))
    DBGTrace1("CryptProtectData failed: %s", GetLastErrorNameStatic());
  if (pw.cbData) {
    SetRegAny(HKLM, RAPASSKEY(RunAsUser), UserName, REG_BINARY, pw.pbData,
              pw.cbData);
    LocalFree(pw.pbData);
  }
}

bool SavedPasswordOk(DWORD SessionId, LPTSTR RunAsUser, LPTSTR UserName) {
  TCHAR Pass[PWLEN + 1] = {0};
  LoadRunAsPassword(RunAsUser, UserName, Pass, PWLEN);
  if (PasswordOK(SessionId, UserName, Pass, true))
    return zero(Pass), true;
  return zero(Pass), false;
}

/////////////////////////////////////////////////////////////////////////////
//
// Logon Dialog
//
/////////////////////////////////////////////////////////////////////////////
typedef struct _LOGONDLGPARAMS {
  LPCTSTR Msg;
  LPCTSTR Hint;
  LPTSTR User;
  LPTSTR Password;
  BOOL UserReadonly;
  BOOL ForceAdminLogon;
  USERLIST Users;
  int TimeOut;
  int MaxTimeOut;
  DWORD UsrFlags;
  BYTE bRunAs;
  LPTSTR RaUser;
  BOOL bFUS; // Fast User switching
  DWORD SessionId;
  _LOGONDLGPARAMS(LPCTSTR M, LPCTSTR H, LPTSTR Usr, LPTSTR Pwd, BOOL RO,
                  BOOL Adm, DWORD UFlags, DWORD S) {
    Msg = M;
    Hint = H;
    User = Usr;
    Password = Pwd;
    UserReadonly = RO;
    ForceAdminLogon = Adm;
    MaxTimeOut = GetCancelTimeOut; // s
    TimeOut = MaxTimeOut;
    UsrFlags = UFlags;
    bRunAs = FALSE;
    RaUser = NULL;
    bFUS = FALSE;
    SessionId = S;
  }
} LOGONDLGPARAMS;

// User Bitmaps:
static void SetUserBitmap(HWND hwnd) {
  LOGONDLGPARAMS *p = (LOGONDLGPARAMS *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (p == 0)
    return;
  TCHAR User[UNLEN + GNLEN + 2] = {0};
  GetDlgItemText(hwnd, IDC_USER, User, UNLEN + GNLEN + 1);
  if (SendDlgItemMessage(hwnd, IDC_USER, CB_GETCOUNT, 0, 0)) {
    int n = (int)SendDlgItemMessage(hwnd, IDC_USER, CB_GETCURSEL, 0, 0);
    if (n != CB_ERR)
      SendDlgItemMessage(hwnd, IDC_USER, CB_GETLBTEXT, n, (LPARAM)&User);
  }
  HBITMAP bm = p->Users.GetUserBitmap(User);
  HWND BmpIcon = GetDlgItem(hwnd, IDC_USERBITMAP);
  DWORD dwStyle = GetWindowLong(BmpIcon, GWL_STYLE) &
                  (~(SS_TYPEMASK | SS_REALSIZEIMAGE | SS_CENTERIMAGE));
  if (bm) {
    SIZE sz = p->Users.GetUserBitmapSize(User);
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
    SendMessage(BmpIcon, STM_SETIMAGE, IMAGE_ICON,
                SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
  }
  // Password:
  if ((p->bRunAs) || (p->bFUS)) {
    CheckDlgButton(hwnd, IDC_RUNASADMIN, 0);
    ShowWindow(GetDlgItem(hwnd, IDC_RUNASADMIN), SW_HIDE);
    if (p->bRunAs) {
      BOOL bAdmin = IsInAdmins(User, p->SessionId);
      if (bAdmin) {
        if (UACEnabled)
          ShowWindow(GetDlgItem(hwnd, IDC_RUNASADMIN), SW_SHOW);
        CheckDlgButton(hwnd, IDC_RUNASADMIN, 1);
      } else if (IsInSuRunners(User, p->SessionId) &&
                 (!GetRestrictApps(User))) {
        bAdmin = TRUE;
        ShowWindow(GetDlgItem(hwnd, IDC_RUNASADMIN), SW_SHOW);
      }
      if (bAdmin && (_tcsicmp(User, p->User) == 0)) {
        CheckDlgButton(hwnd, IDC_RUNASADMIN, 1);
        BOOL PwExpired = PasswordExpired(User);
        BOOL NoNeedPw = g_AdminTStat.AuthenticationId.HighPart ||
                        g_AdminTStat.AuthenticationId.LowPart;
        BOOL PwOk = GetSavePW && (!PwExpired) && NoNeedPw;
        if (PwOk) {
          SetDlgItemText(hwnd, IDC_PASSWORD, _T("****"));
          EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), 0);
          CheckDlgButton(hwnd, IDC_STOREPASS, 0);
          EnableWindow(GetDlgItem(hwnd, IDC_STOREPASS), 0);
          return;
        }
      }
    }
    EnableWindow(GetDlgItem(hwnd, IDC_STOREPASS), GetSavePW && (User[0] != 0));
    TCHAR Pass[PWLEN + 1] = {0};
    if (LoadRunAsPassword(p->User, User, Pass, PWLEN) &&
        PasswordOK(p->SessionId, User, Pass, false)) {
      SetDlgItemText(hwnd, IDC_PASSWORD, Pass);
      EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), false);
      CheckDlgButton(hwnd, IDC_STOREPASS, 1);
    } else {
      SetDlgItemText(hwnd, IDC_PASSWORD, _T(""));
      CheckDlgButton(hwnd, IDC_STOREPASS, 0);
      if ((_tcsicmp(User, _T("SYSTEM")) == 0) &&
          (IsInAdmins(p->User, p->SessionId) ||
           (IsInSuRunners(p->User, p->SessionId) &&
            (!GetRestrictApps(p->User))))) {
        EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), false);
        EnableWindow(GetDlgItem(hwnd, IDC_STOREPASS), false);
      } else
        EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), 1);
    }
    zero(Pass);
  }
}

// Dialog Resizing:
SIZE RectSize(RECT r) {
  SIZE s = {r.right - r.left, r.bottom - r.top};
  return s;
}

SIZE CliSize(HWND w) {
  RECT r = {0};
  GetClientRect(w, &r);
  return RectSize(r);
}

SIZE WindowSize(HWND w) {
  RECT r = {0};
  GetWindowRect(w, &r);
  return RectSize(r);
}

// These controls are not changed on X-Resize
int NX_Ctrls[] = {IDC_SECICON, IDC_SECICON1, IDC_USERBITMAP, IDC_USRST,
                  IDC_PWDST};
// These controls are stretched on X-Resize
int SX_Ctrls[] = {IDC_WHTBK,       IDC_HINTBK,    IDC_FRAME1,   IDC_FRAME2,
                  IDC_DLGQUESTION, IDC_USER,      IDC_PASSWORD, IDC_STOREPASS,
                  IDC_RUNASADMIN,  IDC_HINT,      IDC_HINT2,    IDC_ALWAYSOK,
                  IDC_SHELLEXECOK, IDC_AUTOCANCEL};
// These controls are moved on X-Resize
int MX_Ctrls[] = {IDCANCEL, IDOK};
// These controls are not changed on Y-Resize
int NY_Ctrls[] = {IDC_SECICON};
// These controls are stretched on Y-Resize
int SY_Ctrls[] = {IDC_WHTBK, IDC_DLGQUESTION};
// These controls are moved on Y-Resize
int MY_Ctrls[] = {
    IDC_SECICON1,  IDC_USERBITMAP,  IDC_HINTBK,    IDC_FRAME1, IDC_FRAME2,
    IDC_USER,      IDC_PASSWORD,    IDC_HINT,      IDC_HINT2,  IDCANCEL,
    IDC_STOREPASS, IDC_RUNASADMIN,  IDOK,          IDC_USRST,  IDC_PWDST,
    IDC_ALWAYSOK,  IDC_SHELLEXECOK, IDC_AUTOCANCEL};

void MoveDlgCtrl(HWND hDlg, int nId, int x, int y, int dx, int dy) {
  HWND w = GetDlgItem(hDlg, nId);
  if (w) {
    RECT r;
    GetWindowRect(w, &r);
    ScreenToClient(hDlg, (LPPOINT)&r.left);
    ScreenToClient(hDlg, (LPPOINT)&r.right);
    MoveWindow(w, r.left + x, r.top + y, r.right - r.left + dx,
               r.bottom - r.top + dy, TRUE);
  }
}

void MoveWnd(HWND hwnd, int x, int y, int dx, int dy) {
  RECT r;
  GetWindowRect(hwnd, &r);
  MoveWindow(hwnd, r.left + x, r.top + y, r.right - r.left + dx,
             r.bottom - r.top + dy, TRUE);
}

SIZE GetDrawSize(HWND w) {
  TCHAR s[4096];
  GetWindowText(w, s, 4096);
  RECT r = {0};
  HGDIOBJ font = (HGDIOBJ)SendMessage(w, WM_GETFONT, 0, 0);
  {
    HDC dc = GetDC(0);
    HDC MemDC = CreateCompatibleDC(dc);
    ReleaseDC(0, dc);
    SelectObject(MemDC, font);
    DrawText(MemDC, s, -1, &r,
             DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX | DT_EXPANDTABS);
    DeleteDC(MemDC);
  }
  SIZE S = {r.right - r.left, r.bottom - r.top};
  // Limit dialog size to 90% of the screen width
  int maxDX = GetSystemMetrics(SM_CXFULLSCREEN) * 9 / 10;
  int maxDY = GetSystemMetrics(SM_CYFULLSCREEN) * 9 / 10;
  HWND Parent = GetParent(w);
  {
    SIZE PS = WindowSize(Parent);
    SIZE SS = WindowSize(w);
    maxDX -= PS.cx - SS.cx;
    maxDY -= PS.cy - SS.cy;
  }
  bool bHScroll = S.cx > maxDX;
  bool bVScroll = S.cy > maxDY;
  if (bHScroll || bVScroll) {
    if (bHScroll)
      S.cx = maxDX;
    if (bVScroll)
      S.cy = maxDY;
    // Static controls cannot scroll
    // replace Static with EDIT control to enable Scrolling
    GetWindowRect(w, &r);
    ScreenToClient(Parent, (POINT *)&r.left);
    ScreenToClient(Parent, (POINT *)&r.right);
    DestroyWindow(w);
    w = CreateWindow(L"Edit", s,
                     WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL |
                         ES_MULTILINE | ES_READONLY |
                         (bHScroll ? WS_HSCROLL : 0) |
                         (bVScroll ? WS_VSCROLL : 0),
                     r.left, r.top, r.right - r.left, r.bottom - r.top, Parent,
                     (HMENU)IDC_DLGQUESTION, 0, 0);
    SendMessage(w, WM_SETFONT, (WPARAM)font, 1);
  }
  return S;
}

void SetWindowSizes(HWND hDlg) {
  RECT dlgr = {0};
  GetWindowRect(hDlg, &dlgr);
  SIZE ds = GetDrawSize(GetDlgItem(hDlg, IDC_DLGQUESTION));
  SIZE es = CliSize(GetDlgItem(hDlg, IDC_DLGQUESTION));
  // Resize X
  int dx = ds.cx - es.cx;
  if (dx > 0) {
    MoveWnd(hDlg, -dx / 2, 0, dx, 0);
    int i;
    for (i = 0; i < countof(SX_Ctrls); i++)
      MoveDlgCtrl(hDlg, SX_Ctrls[i], 0, 0, dx, 0);
    for (i = 0; i < countof(MX_Ctrls); i++)
      MoveDlgCtrl(hDlg, MX_Ctrls[i], dx, 0, 0, 0);
  }
  // Resize Y
  int dy = ds.cy - es.cy;
  if (dy > 0) {
    MoveWnd(hDlg, 0, -dy / 2, 0, dy);
    int i;
    for (i = 0; i < countof(SY_Ctrls); i++)
      MoveDlgCtrl(hDlg, SY_Ctrls[i], 0, 0, 0, dy);
    for (i = 0; i < countof(MY_Ctrls); i++)
      MoveDlgCtrl(hDlg, MY_Ctrls[i], 0, dy, 0, 0);
  }
}

// DialogProc
HBRUSH g_HintBrush = 0;
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
    LOGONDLGPARAMS *p = (LOGONDLGPARAMS *)lParam;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
    if (IsWindowEnabled(GetDlgItem(hwnd, IDC_PASSWORD)))
      g_HintBrush = CreateSolidBrush(RGB(192, 128, 0));
    else
      g_HintBrush = CreateSolidBrush(RGB(128, 192, 0));
    SendMessage(hwnd, WM_SETICON, ICON_BIG,
                (LPARAM)LoadImage(GetModuleHandle(0),
                                  MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32,
                                  32, 0));
    SendMessage(hwnd, WM_SETICON, ICON_SMALL,
                (LPARAM)LoadImage(GetModuleHandle(0),
                                  MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16,
                                  16, 0));
    SendDlgItemMessage(hwnd, IDC_DLGQUESTION, WM_SETFONT,
                       (WPARAM)CreateFont(-14, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0,
                                          0, 0, 0, 0, _T("MS Shell Dlg")),
                       1);
    if (GetDlgItem(hwnd, IDC_HINT))
      SendDlgItemMessage(hwnd, IDC_HINT, WM_SETFONT,
                         (WPARAM)CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0,
                                            0, 0, 0, 0, _T("MS Shell Dlg")),
                         1);
    if (p->Msg)
      SetDlgItemText(hwnd, IDC_DLGQUESTION, p->Msg);
    if (p->Hint) {
      SetDlgItemText(hwnd, IDC_HINT, p->Hint);
      if (p->ForceAdminLogon)
        EnableWindow(GetDlgItem(hwnd, IDC_ALWAYSOK), false);
    }
    // #ifndef SuRunEXT_EXPORTS
    // #ifdef DoDBGTrace
    //       {
    //         static TCHAR c1[4096];
    //         _tcscpy(&c1[_tcslen(c1)],L"{\r\n\tElapsed Time from \"SuRun.exe\"
    //         start:"); for (DWORD i=1;i<g_nTimes;i++)
    //         {
    //           _stprintf(&c1[_tcslen(c1)],L"\r\n\t%s\t %d ms",
    //             g_RunTimeNames[i],g_RunTimes[i]-g_RunTimes[0]);
    //         }
    //         SetDlgItemText(hwnd,IDC_DLGQUESTION,CBigResStr(L"%s\r\n%s\r\n\tGUI
    //         visible\t\t %d ms\r\n}",
    //           p->Msg,c1,timeGetTime()-g_RunTimes[0]));
    //       }
    // #endif //DoDBGTrace
    // #endif //SuRunEXT_EXPORTS

    SetDlgItemText(hwnd, IDC_PASSWORD, p->Password);
    SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETPASSWORDCHAR, '*', 0);
    BOOL bFoundUser = FALSE;
    for (int i = 0; i < p->Users.GetCount(); i++) {
      SendDlgItemMessage(hwnd, IDC_USER, CB_INSERTSTRING, i,
                         (LPARAM)p->Users.GetUserName(i));
      if (_tcsicmp(p->Users.GetUserName(i), p->bRunAs ? p->RaUser : p->User) ==
          0) {
        SendDlgItemMessage(hwnd, IDC_USER, CB_SETCURSEL, i, 0);
        bFoundUser = TRUE;
      }
    }
    if (p->UserReadonly) {
      EnableWindow(GetDlgItem(hwnd, IDC_USER), false);
      if (GetNoRunSetup(p->User)) {
        EnableWindow(GetDlgItem(hwnd, IDC_SHELLEXECOK), false);
        EnableWindow(GetDlgItem(hwnd, IDC_ALWAYSOK), false);
      }
    } else if (p->ForceAdminLogon)
      EnableWindow(GetDlgItem(hwnd, IDC_USER), true);
    if (IsWindowEnabled(GetDlgItem(hwnd, IDC_USER)))
      SetFocus(GetDlgItem(hwnd, IDC_USER));
    else if (IsWindowEnabled(GetDlgItem(hwnd, IDC_PASSWORD))) {
      SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETSEL, 0, -1);
      SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
    } else
      SetFocus(GetDlgItem(hwnd, IDCANCEL));
    // Fast User swithing always set IDC_USER Text to Users(0):
    if (p->bFUS)
      SetDlgItemText(hwnd, IDC_USER, p->Users.GetUserName(0));
    else if ((!p->ForceAdminLogon) || p->bRunAs || bFoundUser)
      SetDlgItemText(hwnd, IDC_USER,
                     (bFoundUser && p->bRunAs) ? p->RaUser : p->User);
    else {
      SetDlgItemText(hwnd, IDC_USER, p->Users.GetUserName(0));
      SendDlgItemMessage(hwnd, IDC_USER, CB_SETCURSEL, 0, 0);
    }
    CheckDlgButton(hwnd, IDC_SHELLEXECOK,
                   (p->UsrFlags & FLAG_SHELLEXEC) ? 1 : 0);
    // FLAG_CANCEL_SX: if this is set this dialog will not show up
    // FLAG_AUTOCANCEL: if this is set this dialog will not show up
    CheckDlgButton(hwnd, IDC_ALWAYSOK, (p->UsrFlags & FLAG_DONTASK) ? 1 : 0);
    SendDlgItemMessage(hwnd, IDC_AUTOCANCEL, PBM_SETRANGE, 0,
                       MAKELPARAM(0, p->MaxTimeOut));
    SendDlgItemMessage(hwnd, IDC_AUTOCANCEL, PBM_SETPOS, 0, 0);

    if ((!GetUseIShExHook) && (!GetUseIATHook))
      EnableWindow(GetDlgItem(hwnd, IDC_SHELLEXECOK), 0);
    SetUserBitmap(hwnd);
    SetWindowSizes(hwnd);
    if ((!GetShowCancelTimeOut) || (!GetUseCancelTimeOut)) {
      RECT wr, cr;
      GetWindowRect(hwnd, &wr);
      GetWindowRect(GetDlgItem(hwnd, IDC_AUTOCANCEL), &cr);
      MoveWindow(hwnd, wr.left, wr.top, wr.right - wr.left, cr.top - wr.top,
                 TRUE);
    }
    if (GetUseCancelTimeOut)
      SetTimer(hwnd, 2, 1000, 0);
    return FALSE;
  } // WM_INITDIALOG
  case WM_DESTROY: {
    HGDIOBJ fs = GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ f =
        (HGDIOBJ)SendDlgItemMessage(hwnd, IDC_DLGQUESTION, WM_GETFONT, 0, 0);
    SendDlgItemMessage(hwnd, IDC_DLGQUESTION, WM_SETFONT, (WPARAM)fs, 0);
    if (f)
      DeleteObject(f);
    if (GetDlgItem(hwnd, IDC_HINT)) {
      f = (HGDIOBJ)SendDlgItemMessage(hwnd, IDC_HINT, WM_GETFONT, 0, 0);
      SendDlgItemMessage(hwnd, IDC_HINT, WM_SETFONT, (WPARAM)fs, 0);
      if (f)
        DeleteObject(f);
    }
    return TRUE;
  }
  case WM_NCDESTROY: {
    if (g_HintBrush)
      DeleteObject(g_HintBrush);
    g_HintBrush = 0;
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
    DestroyIcon((HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
    return TRUE;
  } // WM_NCDESTROY
  case WM_CTLCOLORSTATIC: {
    int CtlId = GetDlgCtrlID((HWND)lParam);
    if ((CtlId == IDC_DLGQUESTION) || (CtlId == IDC_SECICON)) {
      SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
      SetBkMode((HDC)wParam, TRANSPARENT);
      return (BOOL)PtrToUlong(GetSysColorBrush(COLOR_3DHILIGHT));
    }
    if ((CtlId == IDC_HINT) || (CtlId == IDC_SECICON1)) {
      SetTextColor((HDC)wParam, RGB(0, 0, 0));
      SetBkMode((HDC)wParam, TRANSPARENT);
      return (BOOL)PtrToUlong(g_HintBrush);
    }
    if (CtlId == IDC_HINTBK) {
      SetTextColor((HDC)wParam, RGB(0, 0, 0));
      return (BOOL)PtrToUlong(g_HintBrush);
    }
    break;
  }
  case WM_TIMER: {
    if (wParam == 1)
      SetUserBitmap(hwnd);
    else if (wParam == 2) {
      LOGONDLGPARAMS *p =
          (LOGONDLGPARAMS *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      p->TimeOut--;
      SendDlgItemMessage(hwnd, IDC_AUTOCANCEL, PBM_SETPOS,
                         min(p->MaxTimeOut - p->TimeOut, p->MaxTimeOut), 0);
      if (p->TimeOut <= 0)
        EndDialog(hwnd, 0);
      else if (p->TimeOut < 10)
        SetDlgItemText(hwnd, IDCANCEL, CResStr(IDS_CANCEL, p->TimeOut));
    }
    return TRUE;
  }
  case WM_COMMAND: {
    switch (wParam) {
    case MAKEWPARAM(IDC_USER, CBN_DROPDOWN):
      SetTimer(hwnd, 1, 250, 0);
      return TRUE;
    case MAKEWPARAM(IDC_USER, CBN_CLOSEUP):
      KillTimer(hwnd, 1);
      return TRUE;
    case MAKEWPARAM(IDC_USER, CBN_SELCHANGE):
    case MAKEWPARAM(IDC_USER, CBN_EDITCHANGE):
      SetUserBitmap(hwnd);
      return TRUE;
    case MAKEWPARAM(IDCANCEL, BN_CLICKED): {
      INT_PTR ExitCode = (IsDlgButtonChecked(hwnd, IDC_ALWAYSOK) << 1) +
                         (IsDlgButtonChecked(hwnd, IDC_SHELLEXECOK) << 2);
      EndDialog(hwnd, ExitCode);
    }
      return TRUE;
    case MAKEWPARAM(IDOK, BN_CLICKED): {
      INT_PTR ExitCode = 1 + (IsDlgButtonChecked(hwnd, IDC_ALWAYSOK) << 1) +
                         (IsDlgButtonChecked(hwnd, IDC_SHELLEXECOK) << 2) +
                         (IsDlgButtonChecked(hwnd, IDC_RUNASADMIN) << 3);
      LOGONDLGPARAMS *p =
          (LOGONDLGPARAMS *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (p->bRunAs && (!IsWindowEnabled(GetDlgItem(hwnd, IDC_STOREPASS)))) {
        p->bRunAs = FALSE;
        ExitCode |= 1 << 4;
        // RunAs /USER SYSTEM
        TCHAR User[UNLEN + GNLEN + 2] = {0};
        GetDlgItemText(hwnd, IDC_USER, User, UNLEN + GNLEN + 1);
        if (_tcsicmp(User, _T("SYSTEM")) == 0)
          ExitCode |= 1 << 5;
      }
      if (p->bRunAs || IsWindowEnabled(GetDlgItem(hwnd, IDC_PASSWORD))) {
        TCHAR User[UNLEN + GNLEN + 2] = {0};
        TCHAR Pass[PWLEN + 1] = {0};
        GetDlgItemText(hwnd, IDC_USER, User, UNLEN + GNLEN + 1);
        GetWindowText((HWND)GetDlgItem(hwnd, IDC_PASSWORD), Pass, PWLEN);
        HANDLE hUser =
            GetUserToken(p->SessionId, User, Pass,
                         p->bFUS || ((!p->ForceAdminLogon) && (!p->bRunAs)));
        if (hUser) {
          if ((p->ForceAdminLogon) && (!IsAdmin(hUser))) {
            zero(Pass);
            CloseHandle(hUser);
            SafeMsgBox(hwnd, CResStr(IDS_NOADMIN, User), CResStr(IDS_APPNAME),
                       MB_ICONINFORMATION);
            SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETSEL, 0, -1);
            SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
          } else {
            if ((p->bRunAs) || (p->bFUS)) {
              if (IsDlgButtonChecked(hwnd, IDC_STOREPASS))
                SaveRunAsPassword(p->User, User, Pass);
              else
                DeleteRunAsPassword(p->User, User);
            }
            if (p->User)
              _tcscpy(p->User, User);
            if (p->Password)
              _tcscpy(p->Password, Pass);
            CloseHandle(hUser);
            zero(Pass);
            EndDialog(hwnd, ExitCode);
          }
        } else {
          SafeMsgBox(hwnd, CBigResStr(IDS_LOGONFAILED), CResStr(IDS_APPNAME),
                     MB_ICONINFORMATION);
          SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETSEL, 0, -1);
          SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
        }
      } else
        EndDialog(hwnd, ExitCode);
      return TRUE;
    } // MAKELPARAM(IDOK,BN_CLICKED)
    } // switch (wParam)
    break;
  } // WM_COMMAND
  } // switch(msg)
  return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// Logon Functions
//
/////////////////////////////////////////////////////////////////////////////

BOOL Logon(DWORD SessionId, LPTSTR User, LPTSTR Password, int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, Password, false, false, 0, SessionId);
  p.Users.SetUsualUsers(SessionId, FALSE);
  return (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_LOGONDLG),
                              0, DialogProc, (LPARAM)&p);
}

DWORD ValidateCurrentUser(DWORD SessionId, LPTSTR User, LPTSTR Password,
                          int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, Password, true, false, 0, SessionId);
  p.Users.Add(User);
  p.Users.LoadUserBitmaps();
  return (DWORD)DialogBoxParam(GetModuleHandle(0),
                               MAKEINTRESOURCE(IDD_LOGONDLG), 0, DialogProc,
                               (LPARAM)&p);
}

BOOL ValidateFUSUser(DWORD SessionId, LPTSTR RunAsUser, LPTSTR User) {
  LOGONDLGPARAMS p(NULL, NULL, RunAsUser, NULL, true, false, 0, SessionId);
  p.Users.Add(User);
  p.Users.LoadUserBitmaps();
  p.bFUS = TRUE;
  return (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_FUSDLG),
                              0, DialogProc, (LPARAM)&p);
}

BOOL RunAsLogon(DWORD SessionId, LPTSTR User, LPTSTR Password, LPTSTR LastUser,
                int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, Password, false, false, 0, SessionId);
  p.Users.SetUsualUsers(SessionId, FALSE);
  p.Users.Add(LastUser);
  p.Users.LoadUserBitmaps();
  p.bRunAs = TRUE;
  p.RaUser = LastUser;
  return (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_RUNASDLG),
                              0, DialogProc, (LPARAM)&p);
}

BOOL LogonAdmin(DWORD SessionId, LPTSTR User, LPTSTR Password, int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, Password, false, true, 0, SessionId);
  p.Users.SetGroupUsers(DOMAIN_ALIAS_RID_ADMINS, SessionId, FALSE);
  return (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_LOGONDLG),
                              0, DialogProc, (LPARAM)&p);
}

BOOL LogonAdmin(DWORD SessionId, int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  TCHAR U[UNLEN + GNLEN + 2] = {0};
  TCHAR P[PWLEN] = {0};
  LOGONDLGPARAMS p(S, NULL, U, P, false, true, 0, SessionId);
  p.Users.SetGroupUsers(DOMAIN_ALIAS_RID_ADMINS, SessionId, FALSE);
  BOOL bRet =
      (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_LOGONDLG), 0,
                           DialogProc, (LPARAM)&p);
  zero(U);
  zero(P);
  return bRet;
}

DWORD LogonCurrentUser(DWORD SessionId, LPTSTR User, LPTSTR Password,
                       DWORD UsrFlags, int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, Password, true, false, UsrFlags, SessionId);
  p.Users.Add(User);
  p.Users.LoadUserBitmaps();
  return (DWORD)DialogBoxParam(GetModuleHandle(0),
                               MAKEINTRESOURCE(IDD_CURUSRLOGON), 0, DialogProc,
                               (LPARAM)&p);
}

DWORD AskCurrentUserOk(DWORD SessionId, LPTSTR User, DWORD UsrFlags, int IDmsg,
                       ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  LOGONDLGPARAMS p(S, NULL, User, _T("******"), true, false, UsrFlags,
                   SessionId);
  p.Users.Add(User);
  p.Users.LoadUserBitmaps();
  return (DWORD)DialogBoxParam(GetModuleHandle(0),
                               MAKEINTRESOURCE(IDD_CURUSRACK), 0, DialogProc,
                               (LPARAM)&p);
}

DWORD ValidateAdmin(DWORD SessionId, int IDmsg, ...) {
  va_list va;
  va_start(va, IDmsg);
  CBigResStr S(IDmsg, va);
  CBigResStr S2(IDS_LOGON_AS_ADMIN);
  TCHAR U[UNLEN + GNLEN + 2] = {0};
  TCHAR P[PWLEN] = {0};
  LOGONDLGPARAMS p(S, S2, U, P, false, true, 0, SessionId);
  p.Users.SetGroupUsers(DOMAIN_ALIAS_RID_ADMINS, SessionId, FALSE);
  BOOL bRet =
      (BOOL)DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_CURUSRLOGON),
                           0, DialogProc, (LPARAM)&p);
  zero(U);
  zero(P);
  return bRet;
}

#ifdef _DEBUG
void TestLogonDlgMain() {
  TCHAR User[4096] = L"Kay\\Kay";
  TCHAR Password[4096] = {0};
  BOOL l;
  l = Logon(0, User, Password, IDS_ASKAUTO, L"Logon");
  DBGTrace2("Logon returned %d: %s", l, GetLastErrorNameStatic());
  l = ValidateCurrentUser(0, User, Password, IDS_ASKOK, L"ValidateCurrentUser");
  DBGTrace2("ValidateCurrentUser returned %d: %s", l, GetLastErrorNameStatic());
  l = ValidateFUSUser(0, User, User);
  DBGTrace2("ValidateFUSUser returned %d: %s", l, GetLastErrorNameStatic());
  l = RunAsLogon(0, User, Password, User, IDS_ASKRUNAS, L"RunAsLogon");
  DBGTrace2("RunAsLogon returned %d: %s", l, GetLastErrorNameStatic());
  l = LogonAdmin(0, IDS_NOADMIN2, L"LogonAdmin1");
  DBGTrace2("LogonAdmin returned %d: %s", l, GetLastErrorNameStatic());
  l = LogonAdmin(0, User, Password, IDS_NOADMIN2, L"LogonAdmin2");
  DBGTrace2("LogonAdmin returned %d: %s", l, GetLastErrorNameStatic());
  l = LogonCurrentUser(0, User, Password, 0, IDS_ASKOK, L"LogonCurrentUser");
  DBGTrace2("LogonCurrentUser returned %d: %s", l, GetLastErrorNameStatic());
  l = AskCurrentUserOk(0, User, 0, IDS_ASKOK, L"AskCurrentUserOk");
  DBGTrace2("AskCurrentUserOk returned %d: %s", l, GetLastErrorNameStatic());
  l = ValidateAdmin(0, IDS_ASKOK, L"ValidateAdmin");
  DBGTrace2("ValidateAdmin returned %d: %s", l, GetLastErrorNameStatic());
}

BOOL TestLogonDlg() {
  INITCOMMONCONTROLSEX icce = {sizeof(icce),
                               ICC_USEREX_CLASSES | ICC_WIN95_CLASSES};
  InitCommonControlsEx(&icce);
  SetLocale(MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), SORT_DEFAULT));
  SafeMsgBox(0, L"German", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
  SafeMsgBox(0, L"English", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(MAKELCID(MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH), SORT_DEFAULT));
  SafeMsgBox(0, L"Dutch", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(MAKELCID(MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH), SORT_DEFAULT));
  SafeMsgBox(0, L"Spanish", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(MAKELCID(MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT), SORT_DEFAULT));
  SafeMsgBox(0, L"Polish", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(MAKELCID(MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH), SORT_DEFAULT));
  SafeMsgBox(0, L"French", L"Locale", MB_OK);
  TestLogonDlgMain();
  SetLocale(
      MAKELCID(MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE), SORT_DEFAULT));
  SafeMsgBox(0, L"Portuguese", L"Locale", MB_OK);
  TestLogonDlgMain();
  return TRUE;
}

#endif //_DEBUG
