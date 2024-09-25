// SuRunC.cpp : Defines the entry point for the console application.
//

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <ShlWapi.h>

#pragma comment(lib,"ShlWapi")


void GetErrorName(int ErrorCode,LPTSTR s)
{
  HMODULE hModule = NULL;
  LPTSTR MessageBuffer=0;
  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM ;
  FormatMessage(dwFormatFlags,hModule,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &MessageBuffer,0,NULL);
  if(hModule != NULL)
    FreeLibrary(hModule);
  _stprintf(s,_T("%d(0x%08X): %s"),ErrorCode,ErrorCode,MessageBuffer);
  LocalFree(MessageBuffer);
}

static TCHAR err[4096];
LPCTSTR GetErrorNameStatic(int ErrorCode)
{
  GetErrorName(ErrorCode,err);
  return err;
}

LPCTSTR GetLastErrorNameStatic()
{
  return GetErrorNameStatic(GetLastError());
}

void TRACEx(LPCTSTR s,...)
{
  TCHAR S[4096]={0};
  int len=0;
  va_list va;
  va_start(va,s);
  if (_vsntprintf(&S[len],4095-len,s,va)>=4096)
    DebugBreak();
  va_end(va);
  OutputDebugString(S);
  _tprintf(S);
}

int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
{
  PROCESS_INFORMATION pi={0};
  STARTUPINFO si={0};
  si.cb	= sizeof(si);
  DWORD MaxLen=GetSystemWindowsDirectory(0,0)+(DWORD)_tcslen(GetCommandLine())+MAX_PATH;
  LPTSTR cmd=(LPTSTR)malloc(sizeof(TCHAR)*MaxLen);
  if (!cmd)
    return -1;
  _tcscpy(cmd,PathGetArgs(GetCommandLine()));
  DWORD dwExCode=-1;
  if (CreateProcess(NULL,cmd,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&si,&pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    GetSystemWindowsDirectory(cmd,MaxLen);
    PathAppend(cmd,L"SuRun.exe");
    PathQuoteSpaces(cmd);
    _stprintf(&cmd[_tcslen(cmd)],L" /CONSPID %d %s",pi.dwProcessId,PathGetArgs(GetCommandLine()));
    PROCESS_INFORMATION Spi={0};
    STARTUPINFO Ssi={0};
    si.cb	= sizeof(si);
    if (CreateProcess(NULL,cmd,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&Ssi,&Spi))
    {
      CloseHandle(Spi.hThread);
      WaitForSingleObject(Spi.hProcess,INFINITE);
      DWORD dwE=-1;
      GetExitCodeProcess(Spi.hProcess,&dwE);
      CloseHandle(Spi.hProcess);
      HANDLE p=OpenProcess(SYNCHRONIZE,0,pi.dwProcessId);
      if(p)
      {
        WaitForSingleObject(p,INFINITE);
        GetExitCodeProcess(p,&dwExCode);
        CloseHandle(p);
      }
    }else
    {
      TRACEx(L"CreateProcess(%s) failed: %s",cmd,GetLastErrorNameStatic());
      HANDLE p=OpenProcess(PROCESS_TERMINATE,0,pi.dwProcessId);
      if(p)
      {
        TerminateProcess(p,-1);
        CloseHandle(p);
      }else
        TRACEx(L"OpenProcess(%d) failed: %s",pi.dwProcessId,GetLastErrorNameStatic());
    }
  }else
    TRACEx(L"CreateProcess(%s) failed: %s",cmd,GetLastErrorNameStatic());
  free(cmd);
  return dwExCode;
}
