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
#define WINVER       0x0500
#include <windows.h>
#include <LMAccess.h>
#include <SHLWAPI.H>
#include <TCHAR.h>
#include "IsAdmin.h"
#include "Helpers.h"
#include "LogonDlg.h"
#include "DBGTrace.H"

#pragma comment(lib,"ShlWapi.lib")
#pragma comment(lib,"advapi32.lib")
#pragma warning(disable : 4996)
//////////////////////////////////////////////////////////////////////////////
// 
// IsAdmin  
// 
// check the token of the calling thread/process to see 
// if the caller belongs to the Administrators group.
//////////////////////////////////////////////////////////////////////////////

BOOL IsAdmin(HANDLE hToken/*=NULL*/) 
{
  BOOL   bReturn   = FALSE;
  PACL   pACL      = NULL;
  PSID   psidAdmin = NULL;
  PSECURITY_DESCRIPTOR psdAdmin = NULL;
  HANDLE hTok=NULL;
  if (hToken==NULL)
  {
    if((OpenThreadToken(GetCurrentThread(),TOKEN_DUPLICATE,FALSE,&hTok)==FALSE)
     &&(OpenProcessToken(GetCurrentProcess(),TOKEN_DUPLICATE,&hTok)==FALSE))
      return FALSE;
    hToken=hTok;
  }
  if (!DuplicateToken(hToken,SecurityImpersonation,&hToken))
    return FALSE;
  if (hTok)
    CloseHandle(hTok);
  __try 
  {
    // Initialize Admin SID and SD
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&SystemSidAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,
      DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&psidAdmin))
      __leave;
    psdAdmin = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (psdAdmin == NULL)
      __leave;
    if (!InitializeSecurityDescriptor(psdAdmin,SECURITY_DESCRIPTOR_REVISION))
      __leave;
    // Compute size needed for the ACL.
    DWORD dwACLSize=sizeof(ACL)+sizeof(ACCESS_ALLOWED_ACE)+GetLengthSid(psidAdmin)-sizeof(DWORD);
    // Allocate memory for ACL.
    pACL = (PACL)LocalAlloc(LPTR,dwACLSize);
    if (pACL == NULL)
      __leave;
    // Initialize the new ACL.
    if (!InitializeAcl(pACL,dwACLSize,ACL_REVISION2))
      __leave;
    // Add the access-allowed ACE to the DACL.
    if (!AddAccessAllowedAce(pACL,ACL_REVISION2,ACCESS_READ | ACCESS_WRITE,psidAdmin))
      __leave;
    // Set our DACL to the SD.
    if (!SetSecurityDescriptorDacl(psdAdmin,TRUE,pACL,FALSE))
      __leave;
    // AccessCheck is sensitive about what is in the SD; set the group and owner.
    SetSecurityDescriptorGroup(psdAdmin,psidAdmin,FALSE);
    SetSecurityDescriptorOwner(psdAdmin,psidAdmin,FALSE);
    if (!IsValidSecurityDescriptor(psdAdmin))
      __leave;
    // Initialize GenericMapping structure even though we won't be using generic rights.
    GENERIC_MAPPING GenericMapping;
    GenericMapping.GenericRead    = ACCESS_READ;
    GenericMapping.GenericWrite   = ACCESS_WRITE;
    GenericMapping.GenericExecute = 0;
    GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
    PRIVILEGE_SET ps;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    DWORD dwStatus;
    if (!AccessCheck(psdAdmin,hToken,ACCESS_READ,&GenericMapping,&ps, 
      &dwStructureSize,&dwStatus,&bReturn)) 
      __leave;
  } __finally 
  {
    // Cleanup 
    if (pACL) 
      LocalFree(pACL);
    if (psdAdmin) 
      LocalFree(psdAdmin);  
    if (psidAdmin) 
      FreeSid(psidAdmin);
    CloseHandle(hToken);
  }
  return bReturn;
}

//////////////////////////////////////////////////////////////////////////////
// 
// Check for Vistas "Split Token"
// 
//////////////////////////////////////////////////////////////////////////////

#ifndef SYSTEM_MANDATORY_LABEL_NO_WRITE_UP
typedef enum _TOKEN_INFORMATION_CLASS_1 
{
    TokenElevationType = TokenOrigin+1,
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
} TOKEN_INFORMATION_CLASS_1, *PTOKEN_INFORMATION_CLASS_1;

typedef enum _TOKEN_ELEVATION_TYPE 
{
    TokenElevationTypeDefault = 1,
    TokenElevationTypeFull,
    TokenElevationTypeLimited,
} TOKEN_ELEVATION_TYPE, *PTOKEN_ELEVATION_TYPE;
#endif SYSTEM_MANDATORY_LABEL_NO_WRITE_UP

bool IsInAdminGroup(HANDLE hToken)
{
  bool bRet=false;
  PTOKEN_GROUPS	ptg = GetTokenGroups(hToken);
  // Initialize Admin SID
  SID_IDENTIFIER_AUTHORITY AdminSidAuthority = SECURITY_NT_AUTHORITY;
  PSID AdminSID = NULL;
  AllocateAndInitializeSid(&AdminSidAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdminSID);
  //Search Admin group in Token
  for(UINT i=0;i<ptg->GroupCount;i++)
    if((ptg->Groups[i].Attributes & (SE_GROUP_ENABLED|SE_GROUP_ENABLED_BY_DEFAULT|SE_GROUP_MANDATORY)) 
      && (IsValidSid(ptg->Groups[i].Sid))
      && (EqualSid(ptg->Groups[i].Sid,AdminSID)))
    {
      bRet=TRUE;
      break;
    }
  FreeSid(AdminSID);
  free(ptg);
  return bRet;
}

BOOL IsSplitAdmin(HANDLE hToken/*=NULL*/)
{
  if (_winmajor<6)
    return FALSE;
  BOOL   bRet = FALSE;
  HANDLE hT=hToken;
  if((hT!=NULL) || OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    TOKEN_ELEVATION_TYPE elevationType;
    DWORD dwSize;
    if (GetTokenInformation(hToken,(TOKEN_INFORMATION_CLASS)TokenElevationType,&elevationType,sizeof(elevationType),&dwSize) && (elevationType==TokenElevationTypeLimited))
    {
      TOKEN_LINKED_TOKEN lt = {0}; 
      dwSize = sizeof(lt); 
      if (GetTokenInformation(hToken,(TOKEN_INFORMATION_CLASS)TokenLinkedToken,&lt,dwSize,&dwSize))
      {
        bRet=IsInAdminGroup(lt.LinkedToken);
        CloseHandle(lt.LinkedToken);
      }
    }
    if (hT==NULL)
      CloseHandle(hToken);
  }
  return bRet;
};

//////////////////////////////////////////////////////////////////////////////
// 
// IsLocalSystem, returns true if you are running as local system
// 
//////////////////////////////////////////////////////////////////////////////

bool IsLocalSystem(HANDLE htok)
{
  bool bIsLocalSystem=false;
  BYTE userSid[256];
  DWORD cb = sizeof(userSid);
  if (GetTokenInformation(htok,TokenUser,userSid,cb,&cb ))
  {
    TOKEN_USER* ptu = (TOKEN_USER*)userSid;
    SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_NT_AUTHORITY;
    void* pLocalSystemSid = 0;
    if (AllocateAndInitializeSid(&ntauth,1,SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,
                                  0,0,&pLocalSystemSid))
    {
      bIsLocalSystem=EqualSid(pLocalSystemSid,ptu->User.Sid)!=0;
      FreeSid(pLocalSystemSid);
    }
  }
	return bIsLocalSystem;
}

bool IsLocalSystem()
{
	HANDLE htok = 0;
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&htok ))
    return DBGTrace1("OpenProcessToken() failed: %s",GetLastErrorNameStatic()),false;
  bool bIsLocalSystem=IsLocalSystem(htok);
  CloseHandle(htok);
	return bIsLocalSystem;
}

bool IsLocalSystem(DWORD ProcessID)
{
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,TRUE,ProcessID?ProcessID:GetCurrentProcessId());
  if (!hProc)
  {
    DBGTrace2("OpenProcess(%d) failed: %s",ProcessID,GetLastErrorNameStatic());
    return false;
  }
  HANDLE hToken;
  bool bRet=false;
  // Open impersonation token for process
  if (OpenProcessToken(hProc,TOKEN_QUERY,&hToken))
  {
    bRet=IsLocalSystem(hToken);
    CloseHandle(hToken);
  }else
    DBGTrace2("OpenProcessToken(ID==%d) failed: %s",ProcessID,GetLastErrorNameStatic());
  CloseHandle(hProc);
	return bRet;
}

//////////////////////////////////////////////////////////////////////////////
//
//  RunAs:
//    Start a process with other user cedentials
//////////////////////////////////////////////////////////////////////////////

BOOL RunAs(LPCWSTR lpCmdLine,LPCWSTR szUser,LPCWSTR szPassword)
{
  PROCESS_INFORMATION pi = {0};
  STARTUPINFOW si = {0};
  si.cb = sizeof(STARTUPINFO);
  WCHAR CurDir[4096];
  GetCurrentDirectoryW(4096,CurDir);
  TCHAR un[2*UNLEN+2]={0};
  TCHAR dn[2*UNLEN+2]={0};
  _tcscpy(un,szUser);
  PathStripPath(un);
  _tcscpy(dn,szUser);
  PathRemoveFileSpec(dn);
  if (!CreateProcessWithLogonW(un,dn,szPassword,1,0,(LPWSTR)lpCmdLine,
        CREATE_UNICODE_ENVIRONMENT,0,CurDir,&si,&pi))
  {
    //MessageBox(0,GetLastErrorNameStatic(),0,0);
    return false;
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return TRUE;
}

BOOL RunAsAdmin(LPCTSTR cmdline,int IDmsg)
{
  //Admin...
  if (IsAdmin())
  {
    DBGTrace1("RunAsAdmin(%s) User is already admin! Just calling CreateProcess",cmdline);
    PROCESS_INFORMATION pi={0};
    STARTUPINFO si={0};
    si.cb	= sizeof(si);
    if (!CreateProcess(NULL,(LPTSTR)cmdline,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi))
      return FALSE;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
  }
  //Vista!
  if (IsSplitAdmin())
  {
    DBGTrace1("RunAsAdmin(%s) User is Split Admin! Just calling ShellExecuteEx(runas)",cmdline);
    TCHAR cmd[4096]={0};
    LPTSTR p=PathGetArgs(cmdline);
    _tcscpy(cmd,cmdline);
    PathRemoveArgs(cmd);
    PathUnquoteSpaces(cmd);
    NetworkPathToUNCPath(cmd);
    SHELLEXECUTEINFO sei={0};
    sei.cbSize=sizeof(sei);
    sei.fMask=SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb=_T("runas");
    sei.lpFile=cmd;
    sei.lpParameters=p;
    sei.nShow=SW_SHOWNORMAL;
    if (ShellExecuteEx(&sei) && sei.hProcess)
    {
      CloseHandle(sei.hProcess);
      return TRUE;
    }
    return FALSE;
  }
  //Standard User: Show Logon Dialog
  TCHAR User[UNLEN+GNLEN+2]={0};
  TCHAR Password[PWLEN+1]={0};
  GetProcessUserName(GetCurrentProcessId(),User);
  BOOL bRet=LogonAdmin((DWORD)-1,User,Password,IDmsg) 
    && RunAs(cmdline,User,Password);
  zero(User);      //Clean sensitive Data
  zero(Password);  //Clean sensitive Data
  return bRet;
}
