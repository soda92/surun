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
#define WINVER 0x0500

#include <Psapi.h>
#include <windows.h>

#include <algorithm>
#include <iterator>
#include <list>

#include "../DBGTrace.h"
#include "../IsAdmin.h"
#include "../Service.h"
#include "../Setup.h"
#include "../UserGroups.h"
#include "../helpers.h"

#pragma comment(lib, "PSAPI.lib")

extern HINSTANCE l_hInst;

extern DWORD l_Groups;
#define l_IsAdmin ((l_Groups & IS_IN_ADMINS) != 0)
#define l_IsSuRunner ((l_Groups & IS_IN_SURUNNERS) != 0)

BOOL g_IATInit = FALSE;

#ifdef _SR32
#define DLLNAME "SuRunExt32"
#else _SR32
#define DLLNAME "SuRunExt"
#endif _SR32

// Function Prototypes:
typedef HMODULE(WINAPI *lpLoadLibraryA)(LPCSTR);
typedef HMODULE(WINAPI *lpLoadLibraryW)(LPCWSTR);
typedef HMODULE(WINAPI *lpLoadLibraryExA)(LPCSTR, HANDLE, DWORD);
typedef HMODULE(WINAPI *lpLoadLibraryExW)(LPCWSTR, HANDLE, DWORD);
typedef FARPROC(WINAPI *lpGetProcAddress)(HMODULE, LPCSTR);
typedef BOOL(WINAPI *lpFreeLibrary)(HMODULE);
typedef VOID(WINAPI *lpFreeLibraryAndExitThread)(HMODULE, DWORD);
typedef BOOL(WINAPI *lpCreateProcessA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
                                       LPSECURITY_ATTRIBUTES, BOOL, DWORD,
                                       LPVOID, LPCSTR, LPSTARTUPINFOA,
                                       LPPROCESS_INFORMATION);
typedef BOOL(WINAPI *lpCreateProcessW)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
                                       LPSECURITY_ATTRIBUTES, BOOL, DWORD,
                                       LPVOID, LPCWSTR, LPSTARTUPINFOW,
                                       LPPROCESS_INFORMATION);

typedef BOOL(WINAPI *lpCreateProcessAsUserA)(HANDLE, LPCSTR, LPSTR,
                                             LPSECURITY_ATTRIBUTES,
                                             LPSECURITY_ATTRIBUTES, BOOL, DWORD,
                                             LPVOID, LPCSTR, LPSTARTUPINFOA,
                                             LPPROCESS_INFORMATION);
typedef BOOL(WINAPI *lpCreateProcessAsUserW)(HANDLE, LPCWSTR, LPWSTR,
                                             LPSECURITY_ATTRIBUTES,
                                             LPSECURITY_ATTRIBUTES, BOOL, DWORD,
                                             LPVOID, LPCWSTR, LPSTARTUPINFOW,
                                             LPPROCESS_INFORMATION);

// TEMP!!!
typedef BOOL(WINAPI *lpShellExecuteExW)(LPSHELLEXECUTEINFOW);
typedef BOOL(WINAPI *lpShellExecuteExA)(LPSHELLEXECUTEINFOA);
typedef HINSTANCE(WINAPI *lpShellExecuteA)(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR,
                                           INT);
typedef HINSTANCE(WINAPI *lpShellExecuteW)(HWND, LPCWSTR, LPCWSTR, LPCWSTR,
                                           LPCWSTR, INT);
// END TEMP!!!

typedef BOOL(WINAPI *lpSwitchDesk)(HDESK);

// Forward decl:
BOOL WINAPI CreateProcA(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
                        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR,
                        LPSTARTUPINFOA, LPPROCESS_INFORMATION);
BOOL WINAPI CreateProcW(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
                        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR,
                        LPSTARTUPINFOW, LPPROCESS_INFORMATION);
// TEMP!!!
BOOL WINAPI ShellExExW(LPSHELLEXECUTEINFOW pei);
BOOL WINAPI ShellExExA(LPSHELLEXECUTEINFOA peiA);
HINSTANCE WINAPI ShellExA(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT);
HINSTANCE WINAPI ShellExW(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);
// END TEMP!!!

BOOL WINAPI CreatePAUA(HANDLE, LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
                       LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR,
                       LPSTARTUPINFOA, LPPROCESS_INFORMATION);
BOOL WINAPI CreatePAUW(HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
                       LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR,
                       LPSTARTUPINFOW, LPPROCESS_INFORMATION);

HMODULE WINAPI LoadLibA(LPCSTR);
HMODULE WINAPI LoadLibW(LPCWSTR);
HMODULE WINAPI LoadLibExA(LPCSTR, HANDLE, DWORD);
HMODULE WINAPI LoadLibExW(LPCWSTR, HANDLE, DWORD);

FARPROC WINAPI GetProcAddr(HMODULE, LPCSTR);

BOOL WINAPI FreeLib(HMODULE);
VOID WINAPI FreeLibAndExitThread(HMODULE, DWORD);

BOOL WINAPI SwitchDesk(HDESK);

static lpGetProcAddress orgGPA = NULL;

typedef std::list<HMODULE> ModList;

ModList g_ModList;

// Hook Descriptor class
class CHookDescriptor {
public:
  LPCSTR DllName;
  LPCSTR apiDllName;
  LPCSTR FuncName;
  PROC newFunc;
  PROC orgFunc;
  LPCSTR HostDll;
  CHookDescriptor(LPCSTR Dll, LPCSTR Win7Dll, LPCSTR Win8Dll, LPCSTR Win81Dll,
                  LPCSTR Win1709Dll, LPCSTR Func, PROC nFunc,
                  LPCSTR Host = NULL) {
    DllName = Dll;
    apiDllName = IsWin1709pp ? Win1709Dll
                 : IsWin81pp ? Win81Dll
                             : (IsWin8 ? Win8Dll : Win7Dll);
    FuncName = Func;
    newFunc = nFunc;
    orgFunc = 0;
    HostDll = Host;
  }
  PROC OrgFunc() {
    if (orgFunc)
      return orgFunc;
    if (!orgGPA)
      orgGPA = GetProcAddr;
    if (apiDllName)
      orgFunc = orgGPA(GetModuleHandleA(apiDllName), FuncName);
    if (!orgFunc)
      orgFunc = orgGPA(GetModuleHandleA(DllName), FuncName);
    return orgFunc;
  }
};

// Standard Hooks: These must be implemented!
// BTW: "api-ms-win-core..." names are defined in apisetschema.dll
static CHookDescriptor hkLdLibA("kernel32.dll",
                                NULL, // Win7
                                NULL, // Win8
                                NULL, // Win81
                                NULL, // Win1709
                                "LoadLibraryA", (PROC)LoadLibA);

static CHookDescriptor hkLdLibW("kernel32.dll",
                                NULL, // Win7
                                NULL, // Win8
                                NULL, // Win81
                                NULL, // Win1709
                                "LoadLibraryW", (PROC)LoadLibW);
static CHookDescriptor
    hkLdLibXA("kernel32.dll",
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win7
              "api-ms-win-core-libraryloader-l1-1-1.dll", // Win8
              "api-ms-win-core-libraryloader-l1-2-0.dll", // Win81
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win1709
              "LoadLibraryExA", (PROC)LoadLibExA);
static CHookDescriptor
    hkLdLibXW("kernel32.dll",
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win7
              "api-ms-win-core-libraryloader-l1-1-1.dll", // Win8
              "api-ms-win-core-libraryloader-l1-2-0.dll", // Win81
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win1709
              "LoadLibraryExW", (PROC)LoadLibExW);
static CHookDescriptor
    hkGetPAdr("kernel32.dll",
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win7
              "api-ms-win-core-libraryloader-l1-1-1.dll", // Win8
              "api-ms-win-core-libraryloader-l1-2-0.dll", // Win81
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win1709
              "GetProcAddress", (PROC)GetProcAddr);
static CHookDescriptor
    hkFreeLib("kernel32.dll",
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win7
              "api-ms-win-core-libraryloader-l1-1-1.dll", // Win8
              "api-ms-win-core-libraryloader-l1-2-0.dll", // Win81
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win1709
              "FreeLibrary", (PROC)FreeLib);
static CHookDescriptor
    hkFrLibXT("kernel32.dll",
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win7
              "api-ms-win-core-libraryloader-l1-1-1.dll", // Win8
              "api-ms-win-core-libraryloader-l1-2-0.dll", // Win81
              "api-ms-win-core-libraryloader-l1-1-0.dll", // Win1709
              "FreeLibraryAndExitThread", (PROC)FreeLibAndExitThread);
// User Hooks:
static CHookDescriptor
    hkCrProcA("kernel32.dll",
              "api-ms-win-core-processthreads-l1-1-0.dll", // Win7
              "api-ms-win-core-processthreads-l1-1-1.dll", // Win8
              "api-ms-win-core-processthreads-l1-1-2.dll", // Win81
              "api-ms-win-core-processthreads-l1-1-0.dll", // Win1709
              "CreateProcessA", (PROC)CreateProcA);
static CHookDescriptor
    hkCrProcW("kernel32.dll",
              "api-ms-win-core-processthreads-l1-1-0.dll", // Win7
              "api-ms-win-core-processthreads-l1-1-1.dll", // Win8
              "api-ms-win-core-processthreads-l1-1-2.dll", // Win81
              "api-ms-win-core-processthreads-l1-1-0.dll", // Win1709
              "CreateProcessW", (PROC)CreateProcW);

static CHookDescriptor hkCrPAUA("advapi32.dll",
                                NULL, // Win7
                                NULL, // Win8
                                NULL, // Win81
                                NULL, // Win1709
                                "CreateProcessAsUserA", (PROC)CreatePAUA);

// Windows XP: only hook calls from umpnpmgr.dll to advapi32.dlls
// "CreateProcessAsUserW"
static CHookDescriptor
    hkCrPAUW("advapi32.dll",
             "api-ms-win-core-processthreads-l1-1-0.dll", // Win7
             "api-ms-win-core-processthreads-l1-1-1.dll", // Win8
             "api-ms-win-core-processthreads-l1-1-2.dll", // Win81
             "api-ms-win-core-processthreads-l1-1-0.dll", // Win1709
             "CreateProcessAsUserW", (PROC)CreatePAUW, "umpnpmgr.dll");

static CHookDescriptor hkSwDesk("user32.dll",
                                NULL, // Win7
                                NULL, // Win8
                                NULL, // Win81
                                NULL, // Win1709
                                "SwitchDesktop", (PROC)SwitchDesk);

// TEMP!!!
static CHookDescriptor hkShExExW("shell32.dll",
                                 NULL, // Win7
                                 NULL, // Win8
                                 NULL, // Win81
                                 NULL, // Win1709
                                 "ShellExecuteExW", (PROC)ShellExExW);
static CHookDescriptor hkShExExA("shell32.dll",
                                 NULL, // Win7
                                 NULL, // Win8
                                 NULL, // Win81
                                 NULL, // Win1709
                                 "ShellExecuteExA", (PROC)ShellExExA);
static CHookDescriptor hkShExW("shell32.dll",
                               NULL, // Win7
                               NULL, // Win8
                               NULL, // Win81
                               NULL, // Win1709
                               "ShellExecuteW", (PROC)ShellExW);
static CHookDescriptor hkShExA("shell32.dll",
                               NULL, // Win7
                               NULL, // Win8
                               NULL, // Win81
                               NULL, // Win1709
                               "ShellExecuteA", (PROC)ShellExA);
// END TEMP!!!

// Functions that, if present in the IAT, cause the module to be hooked
static CHookDescriptor *need_hdt[] = {
    &hkCrProcA,
    &hkCrProcW,
    &hkSwDesk,
    // TEMP!!!
    &hkShExExA,
    &hkShExExW,
    &hkShExA,
    &hkShExW,
// END TEMP!!!
#ifdef _TEST_STABILITY
    &hkCrPAUA,
    &hkCrPAUW,
#endif _TEST_STABILITY
};

// Functions that will be hooked
static CHookDescriptor *hdt[] = {
    &hkLdLibA,
    &hkLdLibW,
    &hkLdLibXA,
    &hkLdLibXW,
    &hkFreeLib,
    &hkFrLibXT,
    &hkCrProcA,
    &hkCrProcW,
    &hkSwDesk,
    // TEMP!!!
    &hkShExExA,
    &hkShExExW,
    &hkShExA,
    &hkShExW,
// END TEMP!!!
#ifdef _TEST_STABILITY
    &hkGetPAdr,
    &hkCrPAUA,
    &hkCrPAUW,
#endif _TEST_STABILITY
};

static CHookDescriptor *sys_need_hdt[] = {
    &hkCrPAUA,  &hkCrPAUW,
#ifdef _TEST_STABILITY
    &hkCrProcA, &hkCrProcW, &hkSwDesk,
// TEMP!!!
//    &hkShExExA,
//    &hkShExExW,
//    &hkShExA,
//    &hkShExW,
// END TEMP!!!
#endif _TEST_STABILITY
};

static CHookDescriptor *sys_hdt[] = {
    &hkCrPAUA,  &hkCrPAUW,
#ifdef _TEST_STABILITY
    &hkLdLibA,  &hkLdLibW,  &hkLdLibXA, &hkLdLibXW, &hkGetPAdr,
    &hkFreeLib, &hkFrLibXT, &hkCrProcA, &hkCrProcW, &hkSwDesk,
// TEMP!!!
//    &hkShExExA,
//    &hkShExExW,
//    &hkShExA,
//    &hkShExW,
// END TEMP!!!
#endif _TEST_STABILITY
};

// relative pointers in PE images
#define RelPtr(_T, pe, rpt) (_T)((DWORD_PTR)(pe) + (DWORD_PTR)(rpt))

// returns true if DllName is one to be hooked up
BOOL DoHookDll(char *DllName, char *HostDll) {
  if (IsBadReadPtr(DllName, 1) || IsBadReadPtr(HostDll, 1))
    return FALSE;
  if (l_IsAdmin) {

    for (int i = 0; i < countof(sys_hdt); i++) {
      if ((((!IsWin7pp) || (sys_hdt[i]->apiDllName == 0)) &&
           (stricmp(sys_hdt[i]->DllName, DllName) == 0)) ||
          (sys_hdt[i]->apiDllName &&
           (stricmp(sys_hdt[i]->apiDllName, DllName) == 0)))
        return (sys_hdt[i]->HostDll == 0) ||
               (stricmp(sys_hdt[i]->HostDll, HostDll) == 0);
    }
  } else {
    for (int i = 0; i < countof(hdt); i++) {
      if ((((!IsWin7pp) || (hdt[i]->apiDllName == 0)) &&
           (stricmp(hdt[i]->DllName, DllName) == 0)) ||
          (hdt[i]->apiDllName && (stricmp(hdt[i]->apiDllName, DllName) == 0)))
        return (hdt[i]->HostDll == 0) ||
               (stricmp(hdt[i]->HostDll, HostDll) == 0);
    }
  }
  return false;
}

// returns newFunc if DllName->ImpName is one to be hooked up
PROC DoHookFn(char *DllName, char *ImpName) {
  if (IsBadReadPtr(DllName, 1) || IsBadReadPtr(ImpName, 1))
    return 0;
  if (*DllName && *ImpName) {
    if (l_IsAdmin) {
      for (int i = 0; i < countof(sys_hdt); i++) {
        if ((((!IsWin7pp) || (sys_hdt[i]->apiDllName == 0)) &&
             (stricmp(sys_hdt[i]->DllName, DllName) == 0)) ||
            (sys_hdt[i]->apiDllName &&
             (stricmp(sys_hdt[i]->apiDllName, DllName) == 0)))
          if (stricmp(sys_hdt[i]->FuncName, ImpName) == 0)
            return sys_hdt[i]->newFunc;
      }
    } else {
      for (int i = 0; i < countof(hdt); i++) {
        if ((((!IsWin7pp) || (hdt[i]->apiDllName == 0)) &&
             (stricmp(hdt[i]->DllName, DllName) == 0)) ||
            (hdt[i]->apiDllName && (stricmp(hdt[i]->apiDllName, DllName) == 0)))
          if (stricmp(hdt[i]->FuncName, ImpName) == 0)
            return hdt[i]->newFunc;
      }
    }
  }
  return false;
}

// Detect if a module is using CreateProcess, if yes, it needs to be hooked
bool NeedHookFn(char *DllName, char *ImpName, void *orgFunc) {
  if (IsBadReadPtr(DllName, 1) || IsBadReadPtr(ImpName, 1) ||
      IsBadReadPtr(ImpName, 1))
    return 0;
  if (*DllName && *ImpName) {
    if (l_IsAdmin) {
      for (int i = 0; i < countof(sys_need_hdt); i++) {
        if ((stricmp(sys_need_hdt[i]->DllName, DllName) == 0) ||
            (sys_need_hdt[i]->apiDllName &&
             (stricmp(sys_need_hdt[i]->apiDllName, DllName) == 0)))
          if (stricmp(sys_need_hdt[i]->FuncName, ImpName) == 0)
            return sys_need_hdt[i]->OrgFunc() == orgFunc;
      }
    } else {
      for (int i = 0; i < countof(need_hdt); i++) {
        if ((stricmp(need_hdt[i]->DllName, DllName) == 0) ||
            (need_hdt[i]->apiDllName &&
             (stricmp(need_hdt[i]->apiDllName, DllName) == 0)))
          if (stricmp(need_hdt[i]->FuncName, ImpName) == 0)
            return need_hdt[i]->OrgFunc() == orgFunc;
      }
    }
  }
  return false;
}

DWORD HookIAT(char *fMod, HMODULE hMod, PIMAGE_IMPORT_DESCRIPTOR pID) {
  DWORD nHooked = 0;
#ifdef DoDBGTrace
//  char fmod[MAX_PATH]={0};
//  {
//    GetModuleFileNameA(0,fmod,MAX_PATH);
//    SR_PathStripPathA(fmod);
//    strcat(fmod,": ");
//    char* p=&fmod[strlen(fmod)];
//    GetModuleFileNameA(hMod,p,MAX_PATH);
//    SR_PathStripPathA(p);
//  }
//  TRACExA("%s: %sHookIAT(%s[%x])\n",(bUnHook?"Un":""),DLLNAME,fmod,hMod);
#endif DoDBGTrace
  for (; pID->Name; pID++) {
    char *DllName = RelPtr(char *, hMod, pID->Name);
    if (DoHookDll(DllName, fMod)) {
      PIMAGE_THUNK_DATA pOrgThunk =
          RelPtr(PIMAGE_THUNK_DATA, hMod, pID->OriginalFirstThunk);
      PIMAGE_THUNK_DATA pThunk =
          RelPtr(PIMAGE_THUNK_DATA, hMod, pID->FirstThunk);
      for (; (pThunk->u1.Function) && (pOrgThunk->u1.Function);
           pOrgThunk++, pThunk++)
        if ((pOrgThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0) {
          PIMAGE_IMPORT_BY_NAME pBN =
              RelPtr(PIMAGE_IMPORT_BY_NAME, hMod, pOrgThunk->u1.AddressOfData);
          PROC newFunc = DoHookFn(DllName, (char *)pBN->Name);
          // PROC newFunc = DoHookFn(DllName,(PROC)pThunk->u1.Function);
          if (newFunc && (pThunk->u1.Function != (DWORD_PTR)newFunc)) {
            __try {
              // MEMORY_BASIC_INFORMATION mbi;
              DWORD oldProt = PAGE_READWRITE;
              if (VirtualProtect(&pThunk->u1.Function,
                                 sizeof(pThunk->u1.Function),
                                 PAGE_EXECUTE_WRITECOPY, &oldProt)) {
#ifdef DoDBGTrace
//                if((newFunc==(PROC)ShellExExW)
//                  ||(newFunc==(PROC)ShellExExA)
//                  ||(newFunc==(PROC)ShellExA)
//                  ||(newFunc==(PROC)ShellExW))
//                  TRACExA("%s: %sHookFunc(%s):%s,%s (%x->%x) newProt:%x;
//                  oldProt:%x\n",DLLNAME,
//                    (bUnHook?"Un":""),fmod,DllName,pBN->Name,pThunk->u1.Function,newFunc,PAGE_EXECUTE_WRITECOPY,oldProt);
#endif DoDBGTrace
                InterlockedExchangePointer((VOID **)&pThunk->u1.Function,
                                           newFunc);
                VirtualProtect(&pThunk->u1.Function,
                               sizeof(pThunk->u1.Function), oldProt, &oldProt);
                nHooked++;
              }
            } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                            ? EXCEPTION_EXECUTE_HANDLER
                            : EXCEPTION_CONTINUE_SEARCH) {
              DBGTrace("FATAL: Exception in HookIAT");
            }
          }
        }
    } // if(DoHookDll(DllName))
  } // for(;pID->Name;pID++)
  return nHooked;
}

DWORD Hook(HMODULE hMod) {
  // Detect if a module is using CreateProcess, if yes, it needs to be hooked
  if (hMod == l_hInst)
    return 0;
  char fMod[MAX_PATH] = {0};
  GetModuleFileNameAEx(hMod, fMod, MAX_PATH);
  {
    char f1[MAX_PATH] = {0};
    GetModuleFileNameAEx(l_hInst, f1, MAX_PATH);
    if (stricmp(fMod, f1) == 0)
      return 0;
  }
  SR_PathStripPathA(fMod);
  // check "MZ" and DOS Header size
  if (IsBadReadPtr(hMod, sizeof(IMAGE_DOS_HEADER)) ||
      (((PIMAGE_DOS_HEADER)hMod)->e_magic != IMAGE_DOS_SIGNATURE))
    return 0;
  // check "PE" and DOS Header size
  PIMAGE_NT_HEADERS pNTH =
      RelPtr(PIMAGE_NT_HEADERS, hMod, ((PIMAGE_DOS_HEADER)hMod)->e_lfanew);
  if (IsBadReadPtr(pNTH, sizeof(IMAGE_NT_HEADERS)) ||
      (pNTH->Signature != IMAGE_NT_SIGNATURE))
    return 0;
  // the right header?
#ifdef _WIN64
  if (pNTH->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
    return 0;
#else _WIN64
  if (pNTH->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
    return 0;
#endif _WIN64
  if (pNTH->FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER))
    return 0;
  // parse IAT
  DWORD_PTR va =
      pNTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
          .VirtualAddress;
  if (va == 0)
    return false;
  PIMAGE_IMPORT_DESCRIPTOR pID = RelPtr(PIMAGE_IMPORT_DESCRIPTOR, hMod, va);
  for (; pID->Name; pID++) {
    char *DllName = RelPtr(char *, hMod, pID->Name);
    if (DoHookDll(DllName, fMod)) {
      PIMAGE_THUNK_DATA pOrgThunk =
          RelPtr(PIMAGE_THUNK_DATA, hMod, pID->OriginalFirstThunk);
      PIMAGE_THUNK_DATA pThunk =
          RelPtr(PIMAGE_THUNK_DATA, hMod, pID->FirstThunk);
      for (; (pThunk->u1.Function) && (pOrgThunk->u1.Function);
           pOrgThunk++, pThunk++)
        if ((pOrgThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0) {
          PIMAGE_IMPORT_BY_NAME pBN =
              RelPtr(PIMAGE_IMPORT_BY_NAME, hMod, pOrgThunk->u1.AddressOfData);
          if (NeedHookFn(DllName, (char *)pBN->Name,
                         (void *)pThunk->u1.Function)) {
            // Hook IAT
            DWORD dwRet = HookIAT(fMod, hMod, pID);
            return dwRet;
          }
        }
    } // if(DoHookDll(DllName))
  } // for(;pID->Name;pID++)
  return false;
}

DWORD HookModules() {
  if (!GetUseIATHook)
    return 0;
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
  if (!hProc)
    return 0;
  DWORD Siz = 0;
  DWORD nHooked = 0;
  EnumProcessModules(hProc, 0, 0, &Siz);
  HMODULE *hMod = Siz ? (HMODULE *)malloc(Siz) : 0;
  if (hMod) {
    DWORD n = Siz / sizeof(HMODULE);
    EnumProcessModules(hProc, hMod, Siz, &Siz);
    std::sort(hMod, hMod + n);
    ModList newMods;
    // get new hModules
    std::set_difference(hMod, hMod + n, g_ModList.begin(), g_ModList.end(),
                        std::back_inserter(newMods));
    // Hook new hModules
    for (ModList::iterator it = newMods.begin(); it != newMods.end(); ++it)
      nHooked += Hook(*it);
    // merge new hModules to list
    g_ModList.merge(newMods);
    free(hMod);
  }
  CloseHandle(hProc);
  return nHooked;
}

CRITICAL_SECTION g_HookCs = {0};

DWORD TestAutoSuRunW(LPCWSTR lpApp, LPWSTR lpCmd, LPCWSTR lpCurDir,
                     DWORD dwCreationFlags, LPPROCESS_INFORMATION lppi,
                     HANDLE hUser = 0) {
  if (!g_IATInit)
    return /*DBGTrace("NoIATHookInit->NoAutoSuRun"),*/ RETVAL_SX_NOTINLIST;
  if (!GetUseIATHook)
    return /*DBGTrace("NoIATHook->NoAutoSuRun"),*/ RETVAL_SX_NOTINLIST;
  DWORD ExitCode = ERROR_ACCESS_DENIED;
  EnterCriticalSection(&g_HookCs);
  static TCHAR CurDir[4096];
  zero(CurDir);
  if (lpCurDir && *lpCurDir)
    _tcsncpy(CurDir, lpCurDir, 4095);
  else
    GetCurrentDirectory(countof(CurDir), CurDir);
  // ToDo: use dynamic allocated strings
  if (StrLenW(lpApp) + StrLenW(CurDir) + StrLenW(lpCmd) > 4095 - 64)
    return LeaveCriticalSection(&g_HookCs), RETVAL_SX_NOTINLIST;
  WCHAR *parms = (lpCmd && wcslen(lpCmd)) ? lpCmd : 0;
  static WCHAR cmd[4096];
  zero(cmd);
  if (lpApp && (*lpApp)) {
    wcscat(cmd, lpApp);
    SR_PathQuoteSpacesW(cmd);
    if (parms)
      // lpApplicationName and the first token of lpCommandLine are the same
      // we need to check this:
      parms = SR_PathGetArgsW(lpCmd);
    if (parms)
      wcscat(cmd, L" ");
  }
  if (parms)
    wcscat(cmd, parms);
  // ToDo: Directly write to service pipe!
  static WCHAR tmp[4096];
  zero(tmp);
  ResolveCommandLine(cmd, CurDir, tmp);
  GetSystemWindowsDirectoryW(cmd, countof(cmd));
  SR_PathAppendW(cmd, L"SuRun.exe");
  SR_PathQuoteSpacesW(cmd);
  if (_wcsnicmp(cmd, tmp, wcslen(cmd)) == 0)
    // Never start SuRun administrative
    return LeaveCriticalSection(&g_HookCs), RETVAL_SX_NOTINLIST;
  // Exit if ShellExecHook failed on "tmp"
  static WCHAR tmp2[4096];
  if (GetRegStr(HKCU, SURUNKEY, L"LastFailedCmd", tmp2, 4096)) {
    RegDelVal(HKCU, SURUNKEY, L"LastFailedCmd");
    if (_tcsicmp(tmp, tmp2) == 0)
      return LeaveCriticalSection(&g_HookCs), RETVAL_SX_NOTINLIST;
  }
  static RET_PROCESS_INFORMATION rpi;
  zero(rpi);
  wsprintf(&cmd[wcslen(cmd)], L" /QUIET /TESTAA %d %p %s",
           GetCurrentProcessId(), &rpi, tmp);
  //  CTimeLog l(L"IATHook TestAutoSuRun(%s)",tmp);
  static STARTUPINFOW si;
  zero(si);
  static PROCESS_INFORMATION pi;
  zero(pi);
  si.cb = sizeof(si);
  BOOL bStarted = FALSE;
  if (hUser == 0) {
    // Start the child process.
    bStarted = ((lpCreateProcessW)hkCrProcW.OrgFunc())(
        NULL, cmd, NULL, NULL, FALSE, 0, NULL, CurDir, &si, &pi);
  } else {
    // Start the child process as user
    bStarted = ((lpCreateProcessAsUserW)hkCrPAUW.OrgFunc())(
        hUser, NULL, cmd, NULL, NULL, FALSE, 0, NULL, CurDir, &si, &pi);
  }
  if (bStarted) {
    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, (DWORD *)&ExitCode);
    CloseHandle(pi.hProcess);
    if (ExitCode == RETVAL_OK) {
      // return a valid PROCESS_INFORMATION!
      if (lppi) {
        lppi->dwProcessId = rpi.dwProcessId;
        lppi->dwThreadId = rpi.dwThreadId;
        if (rpi.dwProcessId) {
          lppi->hProcess =
              OpenProcess(SURUN_PROCESS_ACCESS_FLAGS1, false, rpi.dwProcessId);
          if (!lppi->hProcess)
            DBGTrace3("SuRun TestAutoSuRunW(%s): OpenProcess(%d) failed: %s",
                      cmd, rpi.dwProcessId, GetLastErrorNameStatic());
        } else
          DBGTrace2("SuRun TestAutoSuRunW(%s): Error, returned ProcessID at %p "
                    "is NULL!",
                    cmd, &rpi);
        if (rpi.dwThreadId) {
          lppi->hThread =
              OpenThread(SURUN_THREAD_ACCESS_FLAGS1, false, rpi.dwThreadId);
          if (!lppi->hThread)
            DBGTrace3("SuRun TestAutoSuRunW(%s): OpenThread(%d) failed: %s",
                      cmd, rpi.dwThreadId, GetLastErrorNameStatic());
        } else
          DBGTrace2("SuRun TestAutoSuRunW(%s): Error, returned ThreadID at %p "
                    "is NULL!",
                    cmd, &rpi);
      }
      //      DBGTrace5("IATHook AutoSuRun(%s) success! PID=%d (h=%x); TID=%d
      //      (h=%x)",
      //        cmd,rpi.dwProcessId,rpi.hProcess,
      //        rpi.dwThreadId,rpi.hThread);
    }
  }
  LeaveCriticalSection(&g_HookCs);
  return ExitCode;
}

DWORD TestAutoSuRunA(LPCSTR lpApp, LPSTR lpCmd, LPCSTR lpCurDir,
                     DWORD dwCreationFlags, LPPROCESS_INFORMATION lppi,
                     HANDLE hUser = 0) {
  if (!g_IATInit)
    return RETVAL_SX_NOTINLIST;
  // ToDo: use dynamic allocated strings
  if (StrLenA(lpApp) + StrLenA(lpCurDir) + StrLenA(lpCmd) > 4095 - 64)
    return RETVAL_SX_NOTINLIST;
  EnterCriticalSection(&g_HookCs);
  static WCHAR wApp[4096];
  zero(wApp);
  if (lpApp)
    MultiByteToWideChar(CP_ACP, 0, lpApp, -1, wApp, (int)4096);
  static WCHAR wCmd[4096];
  zero(wCmd);
  if (lpCmd)
    MultiByteToWideChar(CP_ACP, 0, lpCmd, -1, wCmd, (int)4096);
  static WCHAR wCurDir[4096];
  zero(wCurDir);
  if (lpCurDir)
    MultiByteToWideChar(CP_ACP, 0, lpCurDir, -1, wCurDir, (int)4096);
  DWORD dwRet =
      TestAutoSuRunW((lpApp ? wApp : 0), (lpCmd ? wCmd : 0),
                     (lpCurDir ? wCurDir : 0), dwCreationFlags, lppi, hUser);
  LeaveCriticalSection(&g_HookCs);
  return dwRet;
}

BOOL WINAPI CreateProcA(LPCSTR lpApplicationName, LPSTR lpCommandLine,
                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                        BOOL bInheritHandles, DWORD dwCreationFlags,
                        LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                        LPSTARTUPINFOA lpStartupInfo,
                        LPPROCESS_INFORMATION lpProcessInformation) {
#ifdef DoDBGTrace
//   TRACExA("%s: call to
//   CreateProcA(%s,%s)",DLLNAME,lpApplicationName,lpCommandLine);
#endif DoDBGTrace
  DWORD tas = RETVAL_SX_NOTINLIST;
  if ((!l_IsAdmin) && l_IsSuRunner)
    tas = TestAutoSuRunA(lpApplicationName, lpCommandLine, lpCurrentDirectory,
                         dwCreationFlags, lpProcessInformation);
  if (tas == RETVAL_OK)
    return SetLastError(NOERROR), TRUE;
  if (tas == RETVAL_CANCELLED)
    return SetLastError(ERROR_ACCESS_DENIED), FALSE;
  return ((lpCreateProcessA)hkCrProcA.OrgFunc())(
      lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
      bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
      lpStartupInfo, lpProcessInformation);
}

BOOL WINAPI CreateProcW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                        BOOL bInheritHandles, DWORD dwCreationFlags,
                        LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                        LPSTARTUPINFOW lpStartupInfo,
                        LPPROCESS_INFORMATION lpProcessInformation) {
#ifdef DoDBGTrace
//   TRACEx(L"%s: call to
//   CreateProcW(%s,%s)",_T(DLLNAME),lpApplicationName,lpCommandLine);
#endif DoDBGTrace
  DWORD tas = RETVAL_SX_NOTINLIST;
  if ((!l_IsAdmin) && l_IsSuRunner)
    tas = TestAutoSuRunW(lpApplicationName, lpCommandLine, lpCurrentDirectory,
                         dwCreationFlags, lpProcessInformation);
  if (tas == RETVAL_OK)
    return SetLastError(NOERROR), TRUE;
  if (tas == RETVAL_CANCELLED)
    return SetLastError(ERROR_ACCESS_DENIED), FALSE;
  return ((lpCreateProcessW)hkCrProcW.OrgFunc())(
      lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
      bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
      lpStartupInfo, lpProcessInformation);
}

// TEMP!!!
extern HRESULT ShellExtExecute(LPSHELLEXECUTEINFOW pei); // SuRunExt.cpp
BOOL WINAPI ShellExExW(LPSHELLEXECUTEINFOW pei) {
#ifdef DoDBGTrace
//  TRACExA("%s: call to ShellExExW()",DLLNAME);
#endif DoDBGTrace
  HINSTANCE hInstApp = pei->hInstApp;
  HANDLE hProcess = pei->hProcess;
  if (S_OK == ShellExtExecute(pei))
    return TRUE;
  pei->hInstApp = hInstApp;
  pei->hProcess = hProcess;
  return ((lpShellExecuteExW)hkShExExW.OrgFunc())(pei);
}

BOOL WINAPI ShellExExA(LPSHELLEXECUTEINFOA peiA) {
#ifdef DoDBGTrace
//   TRACExA("%s: call to ShellExExA()",DLLNAME);
#endif DoDBGTrace
  if (peiA) {
    SHELLEXECUTEINFOW pei;
    pei.cbSize = sizeof(SHELLEXECUTEINFOW);
    pei.fMask = peiA->fMask;
    pei.hwnd = peiA->hwnd;
    static WCHAR wVerb[4096];
    zero(wVerb);
    MultiByteToWideChar(CP_ACP, 0, peiA->lpVerb, -1, wVerb, (int)4096);
    static WCHAR wFile[4096];
    pei.lpVerb = wVerb;
    zero(wFile);
    MultiByteToWideChar(CP_ACP, 0, peiA->lpFile, -1, wFile, (int)4096);
    static WCHAR wParameters[4096];
    pei.lpFile = wFile;
    zero(wParameters);
    MultiByteToWideChar(CP_ACP, 0, peiA->lpParameters, -1, wParameters,
                        (int)4096);
    pei.lpParameters = wParameters;
    static WCHAR wDirectory[4096];
    zero(wDirectory);
    MultiByteToWideChar(CP_ACP, 0, peiA->lpDirectory, -1, wDirectory,
                        (int)4096);
    pei.lpDirectory = wDirectory;
    pei.nShow = peiA->nShow;
    pei.hInstApp = peiA->hInstApp;
    pei.lpIDList = peiA->lpIDList;
    static WCHAR wClass[4096];
    zero(wClass);
    MultiByteToWideChar(CP_ACP, 0, peiA->lpClass, -1, wClass, (int)4096);
    pei.lpClass = wClass;
    pei.hkeyClass = peiA->hkeyClass;
    pei.dwHotKey = peiA->dwHotKey;
    pei.hIcon = peiA->hIcon;
    pei.hProcess = peiA->hProcess;
    if (S_OK == ShellExtExecute(&pei)) {
      peiA->hProcess = pei.hProcess;
      peiA->hInstApp = pei.hInstApp;
      return TRUE;
    }
  }
  return ((lpShellExecuteExA)hkShExExA.OrgFunc())(peiA);
}

HINSTANCE WINAPI ShellExA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile,
                          LPCSTR lpParameters, LPCSTR lpDirectory,
                          INT nShowCmd) {
#ifdef DoDBGTrace
//   TRACExA("%s: call to ShellExA()",DLLNAME);
#endif DoDBGTrace
  {
    SHELLEXECUTEINFOW pei;
    pei.cbSize = sizeof(SHELLEXECUTEINFOW);
    pei.fMask = 0;
    pei.hwnd = hwnd;
    static WCHAR wVerb[4096];
    zero(wVerb);
    MultiByteToWideChar(CP_ACP, 0, lpOperation, -1, wVerb, (int)4096);
    static WCHAR wFile[4096];
    pei.lpVerb = wVerb;
    zero(wFile);
    MultiByteToWideChar(CP_ACP, 0, lpFile, -1, wFile, (int)4096);
    static WCHAR wParameters[4096];
    pei.lpFile = wFile;
    zero(wParameters);
    MultiByteToWideChar(CP_ACP, 0, lpParameters, -1, wParameters, (int)4096);
    pei.lpParameters = wParameters;
    static WCHAR wDirectory[4096];
    zero(wDirectory);
    MultiByteToWideChar(CP_ACP, 0, lpDirectory, -1, wDirectory, (int)4096);
    pei.lpDirectory = wDirectory;
    pei.nShow = nShowCmd;
    pei.hInstApp = 0;
    pei.lpIDList = 0;
    pei.lpClass = 0;
    pei.hkeyClass = 0;
    pei.dwHotKey = 0;
    pei.hIcon = 0;
    pei.hProcess = 0;
    if (S_OK == ShellExtExecute(&pei))
      return (HINSTANCE)pei.hInstApp;
  }
  return ((lpShellExecuteA)hkShExA.OrgFunc())(
      hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}

HINSTANCE WINAPI ShellExW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile,
                          LPCWSTR lpParameters, LPCWSTR lpDirectory,
                          INT nShowCmd) {
#ifdef DoDBGTrace
//   TRACExA("%s: call to ShellExW()",DLLNAME);
#endif DoDBGTrace
  {
    SHELLEXECUTEINFOW pei;
    pei.cbSize = sizeof(SHELLEXECUTEINFOW);
    pei.fMask = 0;
    pei.hwnd = hwnd;
    pei.lpVerb = lpOperation;
    pei.lpFile = lpFile;
    pei.lpParameters = lpParameters;
    pei.lpDirectory = lpDirectory;
    pei.nShow = nShowCmd;
    pei.hInstApp = 0;
    pei.lpIDList = 0;
    pei.lpClass = 0;
    pei.hkeyClass = 0;
    pei.dwHotKey = 0;
    pei.hIcon = 0;
    pei.hProcess = 0;
    if (S_OK == ShellExtExecute(&pei))
      return (HINSTANCE)pei.hInstApp;
  }
  return ((lpShellExecuteW)hkShExW.OrgFunc())(
      hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}
// END TEMP!!!

static BOOL IsShellAndSuRunner(HANDLE hToken) {
  BOOL bRet = FALSE;
  //  if (l_IsAdmin && (!GetUseSVCHook))
  //    return bRet;
  DWORD Groups = UserIsInSuRunnersOrAdmins(hToken);
  if ((Groups & (IS_IN_ADMINS | IS_IN_SURUNNERS)) == IS_IN_SURUNNERS) {
    DWORD SessionId = 0;
    DWORD siz = 0;
    if (GetTokenInformation(hToken, TokenSessionId, &SessionId, sizeof(DWORD),
                            &siz)) {
      HANDLE hShell = GetSessionUserToken(SessionId);
      if (hShell) {
        PSID ShellSID = GetTokenUserSID(hShell);
        if (ShellSID) {
          PSID UserSID = GetTokenUserSID(hToken);
          if (UserSID) {
            bRet = EqualSid(UserSID, ShellSID);
            // DBGTrace1("IsShellAndSuRunner=%d",bRet);
            free(UserSID);
          }
          free(ShellSID);
        }
        CloseHandle(hShell);
      }
    }
  }
  return bRet;
}

BOOL WINAPI CreatePAUA(HANDLE hToken, LPCSTR lpApplicationName,
                       LPSTR lpCommandLine,
                       LPSECURITY_ATTRIBUTES lpProcessAttributes,
                       LPSECURITY_ATTRIBUTES lpThreadAttributes,
                       BOOL bInheritHandles, DWORD dwCreationFlags,
                       LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                       LPSTARTUPINFOA lpStartupInfo,
                       LPPROCESS_INFORMATION lpProcessInformation) {
  // ToDo: *original function will call CreateProcess. SuRun must not ask twice!
#ifdef DoDBGTrace
//  TRACExA("%s: call to
//  CreatePAUA(%s,%s)",DLLNAME,lpApplicationName,lpCommandLine);
#endif DoDBGTrace
  if (IsShellAndSuRunner(hToken)) {
    DWORD tas =
        TestAutoSuRunA(lpApplicationName, lpCommandLine, lpCurrentDirectory,
                       dwCreationFlags, lpProcessInformation, hToken);
    if (tas == RETVAL_OK)
      return SetLastError(NOERROR), TRUE;
    if (tas == RETVAL_CANCELLED)
      return SetLastError(ERROR_ACCESS_DENIED), FALSE;
  }
  return ((lpCreateProcessAsUserA)hkCrPAUA.OrgFunc())(
      hToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
      lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
      lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL WINAPI CreatePAUW(HANDLE hToken, LPCWSTR lpApplicationName,
                       LPWSTR lpCommandLine,
                       LPSECURITY_ATTRIBUTES lpProcessAttributes,
                       LPSECURITY_ATTRIBUTES lpThreadAttributes,
                       BOOL bInheritHandles, DWORD dwCreationFlags,
                       LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                       LPSTARTUPINFOW lpStartupInfo,
                       LPPROCESS_INFORMATION lpProcessInformation) {
#ifdef DoDBGTrace
//  TRACEx(L"%s: call to
//  CreatePAUW(%s,%s)",_T(DLLNAME),lpApplicationName,lpCommandLine);
#endif DoDBGTrace
  // ToDo: *original function may call CreateProcess. SuRun must not ask twice!
  if (IsShellAndSuRunner(hToken)) {
    DWORD tas =
        TestAutoSuRunW(lpApplicationName, lpCommandLine, lpCurrentDirectory,
                       dwCreationFlags, lpProcessInformation, hToken);
    if (tas == RETVAL_OK)
      return SetLastError(NOERROR), TRUE;
    if (tas == RETVAL_CANCELLED)
      return SetLastError(ERROR_ACCESS_DENIED), FALSE;
  }
  return ((lpCreateProcessAsUserW)hkCrPAUW.OrgFunc())(
      hToken, lpApplicationName, lpCommandLine, lpProcessAttributes,
      lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
      lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

FARPROC WINAPI GetProcAddr(HMODULE hModule, LPCSTR lpProcName) {
  PROC p = 0;
  if (HIWORD(lpProcName)) {
    char f[MAX_PATH] = {0};
    if (GetModuleFileNameA(hModule, f, MAX_PATH)) {
      SR_PathStripPathA(f);
      p = DoHookFn(f, (char *)lpProcName);
#ifdef DoDBGTrace
      if (p)
        TRACExA("%s: intercepting call to GetProcAddr(%s,%s)", DLLNAME, f,
                lpProcName);
#endif DoDBGTrace
      SetLastError(NOERROR);
    }
  }
#ifdef DoDBGTrace
//  else
//  {
//    char f[MAX_PATH]={0};
//    GetModuleFileNameA(hModule,f,MAX_PATH);
//    SR_PathStripPathA(f);
//    TRACExA("%s: call to GetProcAddr(%s,0x%x)",DLLNAME,f,lpProcName);
//  }
#endif DoDBGTrace
  if (!p)
    p = ((lpGetProcAddress)hkGetPAdr.OrgFunc())(hModule, lpProcName);
  return p;
}

HMODULE WINAPI LoadLibA(LPCSTR lpLibFileName) {
#ifdef DoDBGTrace
//    TRACExA("%s: call to LoadLibA(%s)",DLLNAME,lpLibFileName);
#endif DoDBGTrace
  if (g_IATInit) {
    DWORD dwe = GetLastError();
    EnterCriticalSection(&g_HookCs);
    HMODULE hMOD = NULL;
    __try {
      hMOD = ((lpLoadLibraryA)hkLdLibA.OrgFunc())(lpLibFileName);
      if (hMOD)
        HookModules();
    } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
      DBGTrace("FATAL: Exception in LoadLibA");
    }
    LeaveCriticalSection(&g_HookCs);
    SetLastError(dwe);
    return hMOD;
  }
  if (hkLdLibA.OrgFunc())
    return ((lpLoadLibraryA)hkLdLibA.OrgFunc())(lpLibFileName);
  return LoadLibraryA(lpLibFileName);
}

HMODULE WINAPI LoadLibW(LPCWSTR lpLibFileName) {
#ifdef DoDBGTrace
//    TRACEx(L"%s: call to LoadLibW(%s)",_T(DLLNAME),lpLibFileName);
#endif DoDBGTrace
  if (g_IATInit) {
    EnterCriticalSection(&g_HookCs);
    DWORD dwe = GetLastError();
    HMODULE hMOD = NULL;
    __try {
      hMOD = ((lpLoadLibraryW)hkLdLibW.OrgFunc())(lpLibFileName);
      if (hMOD)
        HookModules();
    } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
      DBGTrace("FATAL: Exception in LoadLibW");
    }
    LeaveCriticalSection(&g_HookCs);
    SetLastError(dwe);
    return hMOD;
  }
  if (hkLdLibW.OrgFunc())
    return ((lpLoadLibraryW)hkLdLibW.OrgFunc())(lpLibFileName);
  return LoadLibraryW(lpLibFileName);
}

HMODULE WINAPI LoadLibExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
#ifdef DoDBGTrace
//    TRACExA("%s: call to LoadLibExA(%s)",DLLNAME,lpLibFileName);
#endif DoDBGTrace
  if (g_IATInit) {
    EnterCriticalSection(&g_HookCs);
    DWORD dwe = GetLastError();
    HMODULE hMOD = NULL;
    __try {
      hMOD = ((lpLoadLibraryExA)hkLdLibXA.OrgFunc())(lpLibFileName, hFile,
                                                     dwFlags);
      if (hMOD)
        HookModules();
    } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
      DBGTrace("FATAL: Exception in LoadLibExA");
    }
    LeaveCriticalSection(&g_HookCs);
    SetLastError(dwe);
    return hMOD;
  }
  if (hkLdLibXA.OrgFunc())
    return ((lpLoadLibraryExA)hkLdLibXA.OrgFunc())(lpLibFileName, hFile,
                                                   dwFlags);
  return LoadLibraryExA(lpLibFileName, hFile, dwFlags);
}

HMODULE WINAPI LoadLibExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
#ifdef DoDBGTrace
//    TRACEx(L"%s: call to LoadLibExW(%s)",_T(DLLNAME),lpLibFileName);
#endif DoDBGTrace
  if (g_IATInit) {
    EnterCriticalSection(&g_HookCs);
    DWORD dwe = GetLastError();
    HMODULE hMOD = NULL;
    __try {
      hMOD = ((lpLoadLibraryExW)hkLdLibXW.OrgFunc())(lpLibFileName, hFile,
                                                     dwFlags);
      if (hMOD)
        HookModules();
    } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
      DBGTrace("FATAL: Exception in LoadLibExW");
    }
    LeaveCriticalSection(&g_HookCs);
    SetLastError(dwe);
    return hMOD;
  }
  if (hkLdLibXW.OrgFunc())
    return ((lpLoadLibraryExW)hkLdLibXW.OrgFunc())(lpLibFileName, hFile,
                                                   dwFlags);
  return LoadLibraryExW(lpLibFileName, hFile, dwFlags);
}

BOOL WINAPI FreeLib(HMODULE hLibModule) {
#ifdef DoDBGTrace
//    TRACExA("%s: call to FreeLib()",DLLNAME);
#endif DoDBGTrace
  // The DLL must not be unloaded while the process is running!
  if (hLibModule == l_hInst) {
    if (g_IATInit) {
      SetLastError(NOERROR);
      return true;
    }
  }
  SetLastError(NOERROR);
  BOOL bRet = ((lpFreeLibrary)hkFreeLib.OrgFunc())(hLibModule);
  return bRet;
}

VOID WINAPI FreeLibAndExitThread(HMODULE hLibModule, DWORD dwExitCode) {
  // The DLL must not be unloaded while the process is running!
  if ((!g_IATInit) || (hLibModule != l_hInst)) {
    if (((lpFreeLibraryAndExitThread)hkFrLibXT.OrgFunc()))
      ((lpFreeLibraryAndExitThread)hkFrLibXT.OrgFunc())(hLibModule, dwExitCode);
    else
      FreeLibraryAndExitThread(hLibModule, dwExitCode);
    return;
  }
  SetLastError(NOERROR);
  ExitThread(dwExitCode);
}

BOOL WINAPI SwitchDesk(HDESK Desk) {
#ifdef DoDBGTrace
//  DBGTrace1("%s: call to SwitchDesk()",_T(DLLNAME));
#endif DoDBGTrace
  if ((!l_IsAdmin) && (!IsLocalSystem())) {
    HDESK d = OpenInputDesktop(0, 0, 0);
    if (!d) {
      DBGTrace2("%s: OpenInputDesktop failed: %s; SwitchDeskop interrupted",
                _T(DLLNAME), GetLastErrorNameStatic());
      return FALSE;
    }
    TCHAR dn[4096] = {0};
    DWORD dnl = 4096;
    if (!GetUserObjectInformation(d, UOI_NAME, dn, dnl, &dnl)) {
      CloseDesktop(d);
      DBGTrace2(
          "%s: GetUserObjectInformation failed: %s; SwitchDeskop interrupted2",
          _T(DLLNAME), GetLastErrorNameStatic());
      return FALSE;
    }
    CloseDesktop(d);
    if ((_tcsicmp(dn, _T("Winlogon")) == 0) ||
        (_tcsnicmp(dn, _T("SRD_"), 4) == 0)) {
      SetLastError(ERROR_ACCESS_DENIED);
      CloseDesktop(d);
      DBGTrace2("%s: SwitchDeskop(%s) interrupted3", _T(DLLNAME), dn);
      return FALSE;
    }
    //    DBGTrace2("%s: SwitchDeskop(%s) granted",_T(DLLNAME),dn);
  }
  BOOL bRet = FALSE;
  if (hkSwDesk.OrgFunc())
    bRet = ((lpSwitchDesk)hkSwDesk.OrgFunc())(Desk);
  else
    bRet = SwitchDesktop(Desk);
  return bRet;
}

void LoadHooks() {
  if (g_IATInit)
    return;
  orgGPA = GetProcAddress;
  {
    // Increase Dll load count
    char fmod[MAX_PATH] = {0};
    GetModuleFileNameA(l_hInst, fmod, MAX_PATH);
    ((lpLoadLibraryA)hkLdLibA.OrgFunc())(fmod);
    if (!IsWin2k) {
      HMODULE hMod = 0;
#define GET_MODULE_HANDLE_EX_FLAG_PIN (0x00000001)
      typedef BOOL(WINAPI * GMHExA)(DWORD, LPCSTR, HMODULE *);
      GMHExA GetModuleHandleExA = (GMHExA)GetProcAddress(
          GetModuleHandleA("kernel32.dll"), "GetModuleHandleExA");
      if (GetModuleHandleExA)
        if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN, fmod, &hMod))
          TRACExA("SuRun IAT-Hook: Locking %s failed: %s", fmod,
                  GetLastErrorNameStatic());
    }
  }
  InitializeCriticalSectionAndSpinCount(&g_HookCs, 0x80000000);
  g_IATInit = TRUE;
  if (!GetUseIATHook)
    return;
  EnterCriticalSection(&g_HookCs);
  if (l_IsAdmin) {
    for (int i = 0; i < countof(sys_hdt); i++)
      PROC p = sys_hdt[i]->OrgFunc();
  } else {
    for (int i = 0; i < countof(hdt); i++)
      PROC p = hdt[i]->OrgFunc();
  }
  __try {
    HookModules();
  } __except ((GetExceptionCode() != DBG_PRINTEXCEPTION_C)
                  ? EXCEPTION_EXECUTE_HANDLER
                  : EXCEPTION_CONTINUE_SEARCH) {
    DBGTrace("FATAL: Exception in LoadHooks()");
  }
  LeaveCriticalSection(&g_HookCs);
}
