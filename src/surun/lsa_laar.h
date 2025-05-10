// clang-format off
//go:build ignore
// clang-format on

#pragma once
#include <windows.h>

typedef enum { DelPrivilege = 0, AddPrivilege, HasPrivilege } PrivOp;

BOOL AccountPrivilege(LPTSTR Account, LPTSTR Privilege, PrivOp op);
LPWSTR GetAccountPrivileges(LPWSTR Account);
PSID GetAccountSID(LPWSTR Account);

#define AddAcctPrivilege(a, p) AccountPrivilege(a, p, AddPrivilege)
#define DelAcctPrivilege(a, p) AccountPrivilege(a, p, DelPrivilege)
#define HasAcctPrivilege(a, p) AccountPrivilege(a, p, HasPrivilege)

#define GetSetTime(a) HasAcctPrivilege(a, SE_SYSTEMTIME_NAME)
#define SetSetTime(a, b)                                                       \
  AccountPrivilege(a, SE_SYSTEMTIME_NAME, b ? AddPrivilege : DelPrivilege)
