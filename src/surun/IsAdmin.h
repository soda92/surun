// clang-format off
//go:build ignore
// clang-format on

#pragma once
#include <Windows.h>

// returns true if the hToken/the current Thread belongs to the Administrators
BOOL IsAdmin(HANDLE hToken = NULL);

// returns true if the token is in the administrators group
bool IsInAdminGroup(HANDLE hToken);

// returns true if the user has a non piviliged split token
BOOL IsSplitAdmin(HANDLE hToken = NULL);

// return true if you are running as local system
bool IsLocalSystem(HANDLE htok);
bool IsLocalSystem();
bool IsLocalSystem(DWORD ProcessID);

// Start a process with Admin cedentials and wait until it closed
BOOL RunAsAdmin(LPCTSTR lpCmdLine, int IDmsg);
