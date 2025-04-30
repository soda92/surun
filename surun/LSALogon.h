// clang-format off
//go:build ignore
// clang-format on
#pragma once
#include <windows.h>

HANDLE LSALogon(DWORD SessionID, LPWSTR UserName, LPWSTR Domain,
                LPWSTR Password, bool bNoAdmin);

HANDLE LogonAsAdmin(LPTSTR UserName, LPTSTR p);

void RestoreUserPasswords();
void DeleteTempAdminTokens();
void DeleteTempAdminToken(HANDLE hToken);
void DeleteTempAdminToken(LPTSTR UserName);
HANDLE GetTempAdminToken(LPTSTR UserName);

HANDLE GetAdminToken(DWORD SessionID);
