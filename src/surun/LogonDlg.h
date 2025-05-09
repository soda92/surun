// clang-format off
//go:build ignore
// clang-format on

//////////////////////////////////////////////////////////////////////////////
// Display various Dialogs to:
// *Logon a Windows User
// *Logon an Administrator
// *Logon the current user
// *Ask, if something should be started with administrative rights
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>

// Check is a password for a user is correct
BOOL PasswordOK(DWORD SessionId, LPCTSTR User, LPCTSTR Password,
                bool AllowEmptyPassword);

BOOL Logon(DWORD SessionId, LPTSTR User, LPTSTR Password, int IDmsg, ...);
DWORD ValidateCurrentUser(DWORD SessionId, LPTSTR User, LPTSTR Password,
                          int IDmsg, ...);

BOOL ValidateFUSUser(DWORD SessionId, LPTSTR RunAsUser, LPTSTR User);

BOOL RunAsLogon(DWORD SessionId, LPTSTR User, LPTSTR Password, LPTSTR LastUser,
                int IDmsg, ...);

BOOL LogonAdmin(DWORD SessionId, LPTSTR User, LPTSTR Password, int IDmsg, ...);
BOOL LogonAdmin(DWORD SessionId, int IDmsg, ...);

DWORD LogonCurrentUser(DWORD SessionId, LPTSTR User, LPTSTR Password,
                       DWORD UsrFlags, int IDmsg, ...);

DWORD AskCurrentUserOk(DWORD SessionId, LPTSTR User, DWORD UsrFlags, int IDmsg,
                       ...);

DWORD ValidateAdmin(DWORD SessionId, int IDmsg, ...);

bool SavedPasswordOk(DWORD SessionId, LPTSTR RunAsUser, LPTSTR UserName);

#ifdef _DEBUG
BOOL TestLogonDlg();
#endif //_DEBUG
