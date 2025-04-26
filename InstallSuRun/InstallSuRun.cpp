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
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>
#include <Aclapi.h>
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib, "version.lib")

typedef UINT (WINAPI* GETSYSWOW64DIRA)(LPSTR, UINT);

#include <lmerr.h>
#pragma comment(lib,"Shlwapi.lib")
#pragma warning(disable : 4996)

void GetErrorName(int ErrorCode,LPTSTR s)
{
  HMODULE hModule = NULL;
  LPTSTR MessageBuffer=0;
  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM ;
  if(ErrorCode >= NERR_BASE && ErrorCode <= MAX_NERR) 
  {
    hModule = LoadLibraryEx("netmsg.dll",NULL,LOAD_LIBRARY_AS_DATAFILE);
    if(hModule != NULL)
      dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }
  FormatMessage(dwFormatFlags,hModule,ErrorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &MessageBuffer,0,NULL);
  if(hModule != NULL)
    FreeLibrary(hModule);
  sprintf(s,"%d(0x%08X): %s",ErrorCode,ErrorCode,MessageBuffer);
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

BOOL SetRegAny(HKEY HK,LPCTSTR SubKey,LPCTSTR ValName,DWORD Type,BYTE* Data,DWORD nBytes)
{
  HKEY Key;
  BOOL bKey=FALSE;
  bKey=RegOpenKeyEx(HK,SubKey,0,KEY_WRITE,&Key)==ERROR_SUCCESS;
  if (!bKey)
    bKey=RegCreateKeyEx(HK,SubKey,0,0,0,KEY_WRITE,0,&Key,0)==ERROR_SUCCESS;
  if (bKey)
  {
    LONG l=RegSetValueEx(Key,ValName,0,Type,Data,nBytes);
    RegCloseKey(Key);
    return l==ERROR_SUCCESS;
  }
  return FALSE;
}

BOOL SetRegStr(HKEY HK,LPCTSTR SubKey,LPCTSTR ValName,LPCTSTR Value)
{
  return SetRegAny(HK,SubKey,ValName,REG_SZ,(BYTE*)Value,(DWORD)strlen(Value)*sizeof(TCHAR));
}

void AllowAccess(LPTSTR FileName)
{
  DWORD dwRes;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea;
  SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
  PSID pEveryoneSID = NULL;
  if (NULL == FileName) 
    return;
  // Get a pointer to the existing DACL.
  dwRes = GetNamedSecurityInfo(FileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,  NULL, NULL, &pOldDACL, NULL, &pSD);
  if (ERROR_SUCCESS != dwRes) 
    goto Cleanup; 
  // Create a well-known SID for the Everyone group.
  if(! AllocateAndInitializeSid( &SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID) ) 
    return;
  // Initialize an EXPLICIT_ACCESS structure for an ACE.
  // The ACE will allow Users read access to the key.
  memset(&ea,0,sizeof(EXPLICIT_ACCESS));
  ea.grfAccessPermissions = FILE_ALL_ACCESS;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;
  // Create a new ACL that merges the new ACE
  // into the existing DACL.
  dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
  if (ERROR_SUCCESS != dwRes)  
    goto Cleanup; 
  // Attach the new ACL as the object's DACL.
  dwRes = SetNamedSecurityInfo(FileName, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,  NULL, NULL, pNewDACL, NULL);
  if (ERROR_SUCCESS != dwRes)  
    goto Cleanup; 
Cleanup:
  if(pSD != NULL) 
    LocalFree((HLOCAL) pSD); 
  if(pNewDACL != NULL) 
    LocalFree((HLOCAL) pNewDACL); 
  if(pEveryoneSID)
    FreeSid(pEveryoneSID);
}

BOOL IsWow64()
{
  GETSYSWOW64DIRA gSysW64Dir;
  HMODULE hKrnl = GetModuleHandle("kernel32.dll");
  if (hKrnl == NULL)
    return FALSE;
  char Dir[MAX_PATH];
  gSysW64Dir=(GETSYSWOW64DIRA)GetProcAddress(hKrnl,"GetSystemWow64DirectoryA");
  if (gSysW64Dir==NULL)
    return FALSE;
  return (gSysW64Dir(Dir,MAX_PATH)!=0)
       ||(GetLastError()!=ERROR_CALL_NOT_IMPLEMENTED);
}

bool ResToTmp(LPCTSTR Section,LPCTSTR ResName)
{
  HMODULE hMod=GetModuleHandle(0);
  HRSRC hResFnd =FindResource(hMod,ResName,Section);
  if (hResFnd==0) 
    return false;
  HGLOBAL hResLd =LoadResource(hMod,hResFnd);
  if (hResLd==0) 
    return false;
  LPVOID pRes=LockResource(hResLd);
  if (pRes ==NULL) 
    return FreeResource(hResLd),false;
  DWORD Siz=SizeofResource(hMod,hResFnd);
  CHAR tmp[4096];
  GetTempPath(MAX_PATH,tmp);
  PathRemoveBackslash(tmp);
  PathAppend(tmp,"SuRunInst");
  CreateDirectory(tmp,0);
  AllowAccess(tmp);
  PathAppend(tmp,ResName);
  FILE* f=fopen(tmp,"wb");
  if (!f)
    return UnlockResource(hResLd),FreeResource(hResLd),false;
  bool res=fwrite(pRes,Siz,1,f)==1;
  fclose(f);
  AllowAccess(tmp);
  UnlockResource(hResLd);
  FreeResource(hResLd);
  return res;
};

void RunTmp(LPSTR cmd,LPCTSTR args=0)
{
  CHAR tmp[4096];
  GetTempPath(MAX_PATH,tmp);
  PathRemoveBackslash(tmp);
  PathAppend(tmp,"SuRunInst");
  SetCurrentDirectory(tmp);
  PathAppend(tmp,cmd);
  if (args)
  {
    PathQuoteSpaces(tmp);
    strcat(tmp," ");
    strcat(tmp,args);
  }
  PROCESS_INFORMATION pi={0};
  STARTUPINFO si={0};
  si.cb	= sizeof(si);
  if (CreateProcess(NULL,tmp,0,0,FALSE,NORMAL_PRIORITY_CLASS,0,0,&si,&pi))
  {
    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess,INFINITE);
    CloseHandle(pi.hProcess);
  }else
  {
    MessageBox(0,GetLastErrorNameStatic(),0,MB_SYSTEMMODAL);
  }
}

void DelTmpFiles()
{
  CHAR tmp[4096];
  CHAR s[4096];
  GetTempPath(MAX_PATH,tmp);
  PathRemoveBackslash(tmp);
  PathAppend(tmp,"SuRunInst");
  PathUnquoteSpaces(tmp);
  //sprintf(s,"cmd.exe /c rd /s /q %s",tmp);
  sprintf(s,"%%SystemRoot%%\\System32\\RunDll32.Exe %%SystemRoot%%\\System32\\AdvPack.Dll,DelNodeRunDLL32 \"%s\"",tmp);
  SetRegStr(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce","Clean_SuRunInst_tmp",s);
}

bool GetVersionStringFileInfoString(LPTSTR FName, LPCTSTR WhatString,LPTSTR sRet)
{
  bool bRet=false;
  int cbVerInfo = GetFileVersionInfoSize(FName, 0);
  if (cbVerInfo)
  {
    void* VerInfo = malloc(cbVerInfo);
    GetFileVersionInfo(FName, 0, cbVerInfo, VerInfo);
    if (VerInfo)
    {
      struct LANGANDCODEPAGE 
      {
        WORD wLanguage;
        WORD wCodePage;
      } *lpTranslate;
      UINT cbTranslate=0;
      // Read the list of languages and code pages.
      VerQueryValue(VerInfo, _T("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);
      if (cbTranslate)
      {
        TCHAR SubBlock[4096];
        // Read the file description for each language and code page.
        _stprintf(SubBlock,_T("\\StringFileInfo\\%04x%04x\\%s"),lpTranslate->wLanguage,lpTranslate->wCodePage,WhatString);
        // Retrieve file description for language and code page "i". 
        LPTSTR lpBuffer=0;
        UINT dwBytes=0;
        VerQueryValue(VerInfo, SubBlock, (LPVOID*)&lpBuffer, &dwBytes); 
        if (dwBytes)
        {
          bRet=true;
          _tcscpy(sRet,lpBuffer);
        }
      }
      free(VerInfo);
    }
  }
  return bRet;
}

int APIENTRY WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
  LPTSTR args=PathGetArgs(GetCommandLine());
  if (_tcsicmp(args,_T("/RenameYourselfToYourVersion"))==0)
  {
    TCHAR OldName[MAX_PATH] = {0};
    TCHAR NewName[MAX_PATH] = {0};
    GetModuleFileName(0, OldName, MAX_PATH);
    _tcscpy(NewName,OldName);
    PathRemoveFileSpec(NewName);
    TCHAR IntName[MAX_PATH] = {0};
    if (GetVersionStringFileInfoString(OldName, _T("InternalName"), IntName))
    {
      PathAppend(NewName,IntName);
      PathAddExtension(NewName,_T(".exe"));
      MoveFile(OldName,NewName);
    }
    return 0;
  }
  {
    //Enable DEP
    HMODULE hMod=GetModuleHandle(_T("Kernel32.dll"));
    if (hMod) 
    {
      typedef BOOL (WINAPI *PSETDEP)(DWORD);
      #define PROCESS_DEP_ENABLE 0x00000001
      PSETDEP SetProcessDEPPolicy=(PSETDEP)GetProcAddress(hMod,"SetProcessDEPPolicy");
      if (SetProcessDEPPolicy) 
        SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
    }
  }
  if(IsWow64()) //Win64
  {
    if ((!ResToTmp("EXE64_FILE","SuRun.exe"))
      ||(!ResToTmp("EXE64_FILE","SuRunExt.dll"))
      ||(!ResToTmp("EXE64_FILE","SuRun32.bin"))
      ||(!ResToTmp("EXE64_FILE","SuRunExt32.dll")))
      return -1;
  }else 
  {
    if ((!ResToTmp("EXE_FILE","SuRun.exe"))
      ||(!ResToTmp("EXE_FILE","SuRunExt.dll")))
      return -1;
  }
  if (args && *args != '\0')
    RunTmp("SuRun.exe",args);
  else
    RunTmp("SuRun.exe /USERINST");
  DelTmpFiles();
	return 0;
}
