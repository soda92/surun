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

#pragma once

typedef enum {
    DelPrivilege = 0,
    AddPrivilege,
    HasPrivilege
}PrivOp;

BOOL AccountPrivilege(LPTSTR Account,LPTSTR Privilege,PrivOp op);
LPWSTR GetAccountPrivileges(LPWSTR Account);
PSID GetAccountSID(LPWSTR Account);

#define AddAcctPrivilege(a,p) AccountPrivilege(a,p,AddPrivilege)
#define DelAcctPrivilege(a,p) AccountPrivilege(a,p,DelPrivilege)
#define HasAcctPrivilege(a,p) AccountPrivilege(a,p,HasPrivilege)

#define GetSetTime(a)   HasAcctPrivilege(a,SE_SYSTEMTIME_NAME)
#define SetSetTime(a,b) AccountPrivilege(a,SE_SYSTEMTIME_NAME,b?AddPrivilege:DelPrivilege)
