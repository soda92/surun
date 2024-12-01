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

// All service related stuff is handled in Service.cpp
// The service bootstraps itself before WinMain gats called

#pragma once
#include <LMCons.h>

// PROCESS_INFORMATION data size is different on x64 and x86
typedef struct {
  DWORD dwProcessId;
  DWORD dwThreadId;
} RET_PROCESS_INFORMATION;

// This is the pipe named used to get the SuRun command to the service
#define ServicePipeName _T("\\\\.\\Pipe\\SuperUserRun")

// This structure is passed from SuRun.exe to the service
// RUNDATA is bigger than required, but I'll leave it this way
#pragma pack(push)
#pragma pack(1)
typedef struct {
  DWORD CliProcessId;
  DWORD CliThreadId;
  DWORD SessionID;
  WCHAR WinSta[MAX_PATH];
  WCHAR Desk[MAX_PATH];
  WCHAR UserName[UNLEN + UNLEN + 2];
  WCHAR cmdLine[4096];
  WCHAR CurDir[4096];
  DWORD KillPID; // SuRun->Service: Process Id to be killed
  DWORD RetPID;  // SuRun->Service: Return PROCESS_INFORMATION to this Process
  unsigned __int64
      RetPtr;      // SuRun->Service: Return PROCESS_INFORMATION to this Address
  DWORD NewPID;    // Service->SuRun: Process Id of the newly started process
  int IconId;      // Service->Tray Warning: ICON Resource Id
  DWORD TimeOut;   // Service->Tray Window timeout
  bool bShlExHook; // we're run from a hook
  bool beQuiet;    // No message boxes
  BYTE bRunAs; // Bit0=1:do a RunAs Logon; Bit1=1: start with low privileges;
               // Bit2=1: RunAs Username given
  DWORD Groups;         // IS_IN_ADMINS,IS_IN_SURUNNERS
  bool bShExNoSafeDesk; // if a safe desktop is required, cancel the request
  // DWORD ConsolePID;     //SuRunC initiated SuRun and the Main Thread of
  // ConsolePID is waiting to be resumed
} RUNDATA;

#define g_CliIsInAdmins ((g_RunData.Groups & IS_IN_ADMINS) != 0)
#define g_CliIsSplitAdmin ((g_RunData.Groups & IS_SPLIT_ADMIN) != 0)
#define g_CliIsInSuRunners ((g_RunData.Groups & IS_IN_SURUNNERS) != 0)

extern bool g_CliIsAdmin;

// This is used to verify that SuRun.exe started by the user is the same as
// the Service process

#define RETVAL_NODESKTOP -2
#define RETVAL_WAIT -1
#define RETVAL_OK 0
#define RETVAL_ACCESSDENIED 1
#define RETVAL_SX_NOTINLIST 2
#define RETVAL_RESTRICT 3
#define RETVAL_CANCELLED 4
extern int g_RetVal;

bool HandleServiceStuff();

// Create...
#define SURUN_PROCESS_ACCESS_FLAGS                                             \
  (SYNCHRONIZE | READ_CONTROL | PROCESS_QUERY_INFORMATION |                    \
   PROCESS_QUERY_LIMITED_INFORMATION)
#define SURUN_THREAD_ACCESS_FLAGS (SYNCHRONIZE)
// Open...
#define SURUN_PROCESS_ACCESS_FLAGS1                                            \
  (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION)
#define SURUN_THREAD_ACCESS_FLAGS1 (SYNCHRONIZE)

#pragma pack(pop)