#define _WIN32_WINNT 0x0500
#define WINVER       0x0500
#include <windows.h>
#include <tchar.h>
#include "DynWTSAPI.h"

HINSTANCE g_wtsapi32=NULL;

BOOL WINAPI WTSQueryUserToken(ULONG SessionId, PHANDLE phToken)
{
  typedef BOOL (WINAPI* wtsqut)(ULONG,PHANDLE);
  static wtsqut wtsqueryusertoken=NULL;
  if (!wtsqueryusertoken)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtsqueryusertoken=(wtsqut)GetProcAddress(g_wtsapi32,"WTSQueryUserToken");
  }
  if (!wtsqueryusertoken)
    return SetLastError(ERROR_INVALID_FUNCTION),false;
  return wtsqueryusertoken(SessionId,phToken);
}

BOOL WINAPI WTSEnumerateProcesses(HANDLE hServer,DWORD Reserved,DWORD Version,PWTS_PROCESS_INFO * ppProcessInfo,DWORD * pCount)
{
  typedef BOOL (WINAPI* wtsenpr)(HANDLE,DWORD,DWORD,PWTS_PROCESS_INFO*,DWORD*);
  static wtsenpr wtsenumerateprocesses=NULL;
  if (!wtsenumerateprocesses)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtsenumerateprocesses=(wtsenpr)GetProcAddress(g_wtsapi32,"WTSEnumerateProcessesW");
  }
  if (!wtsenumerateprocesses)
    return SetLastError(ERROR_INVALID_FUNCTION),false;
  return wtsenumerateprocesses(hServer,Reserved,Version,ppProcessInfo,pCount);
}

BOOL WINAPI WTSEnumerateSessions(HANDLE hServer,DWORD Reserved,DWORD Version,PWTS_SESSION_INFO * ppSessionInfo,DWORD * pCount)
{
  typedef BOOL (WINAPI* wtsenses)(HANDLE,DWORD,DWORD,PWTS_SESSION_INFO*,DWORD*);
  static wtsenses wtsenumeratesessions=NULL;
  if (!wtsenumeratesessions)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtsenumeratesessions=(wtsenses)GetProcAddress(g_wtsapi32,"WTSEnumerateSessionsW");
  }
  if (!wtsenumeratesessions)
    return SetLastError(ERROR_INVALID_FUNCTION),false;
  return wtsenumeratesessions(hServer,Reserved,Version,ppSessionInfo,pCount);
}

BOOL WINAPI WTSQuerySessionInformation(HANDLE hServer,DWORD SessionId,WTS_INFO_CLASS WTSInfoClass,LPWSTR * ppBuffer,DWORD * pBytesReturned)
{
  typedef BOOL (WINAPI* wtsqsi)(HANDLE ,DWORD ,WTS_INFO_CLASS ,LPWSTR * ,DWORD * );
  static wtsqsi wtsquerysessioninformation=NULL;
  if (!wtsquerysessioninformation)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtsquerysessioninformation=(wtsqsi)GetProcAddress(g_wtsapi32,"WTSQuerySessionInformationW");
  }
  if (!wtsquerysessioninformation)
    return SetLastError(ERROR_INVALID_FUNCTION),false;
  return wtsquerysessioninformation(hServer,SessionId,WTSInfoClass,ppBuffer,pBytesReturned);
}

BOOL WINAPI WTSLogoffSession(HANDLE hServer,DWORD SessionId,BOOL bWait)
{
  typedef BOOL (WINAPI* wtsloffs)(HANDLE,DWORD,BOOL);
  static wtsloffs wtslogoffsession=NULL;
  if (!wtslogoffsession)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtslogoffsession=(wtsloffs)GetProcAddress(g_wtsapi32,"WTSLogoffSession");
  }
  if (!wtslogoffsession)
    return SetLastError(ERROR_INVALID_FUNCTION),false;
  return wtslogoffsession(hServer,SessionId,bWait);
}

VOID WINAPI WTSFreeMemory(PVOID pMemory)
{
  typedef VOID (WINAPI* wtsfm)(PVOID);
  static wtsfm wtsfreememory=NULL;
  if (!wtsfreememory)
  {
    if (!g_wtsapi32)
      g_wtsapi32=LoadLibrary(_T("wtsapi32.dll"));
    if (g_wtsapi32)
      wtsfreememory=(wtsfm)GetProcAddress(g_wtsapi32,"WTSFreeMemory");
  }
  if (!wtsfreememory)
    SetLastError(ERROR_INVALID_FUNCTION);
  else
    wtsfreememory(pMemory);
}

