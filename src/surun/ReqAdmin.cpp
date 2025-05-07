// clang-format off
//go:build ignore
// clang-format on
#define _WIN32_WINNT 0x0A00
#define WINVER 0x0A00
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include "DBGTrace.h"
#include "Setup.h"

#pragma comment(lib, "shlwapi.lib")

#define InfoDBGTrace(msg)                                                      \
  { (void)0; }
#define InfoDBGTrace1(msg, d1)                                                 \
  { (void)0; }
#define InfoDBGTrace2(msg, d1, d2)                                             \
  { (void)0; }

// Define this in any implementation, before "pugxml.h", to be notified of API
// campatibility.
#define PUGAPI_VARIANT                                                         \
  0x58475550 // The Pug XML library variant we are using in this implementation.
#define PUGAPI_VERSION_MAJOR                                                   \
  1 // The Pug XML library major version we are using in this implementation.
#define PUGAPI_VERSION_MINOR                                                   \
  2 // The Pug XML library minor version we are using in this implementation.
// Include the Pug XML library.
#include "pugxml.h"

using namespace std;
using namespace pug;

BOOL RequiresAdmin(xml_node document) {
  BOOL bReqAdmin = FALSE;
  xml_node xn = document.first_element_by_name(_T("*trustInfo"));
  if (!xn.empty()) {
    TCHAR name[4096];
    _tcscpy(name, xn.name());
    LPTSTR p = _tcschr(name, ':'); // p point to name after ":" is any
    if (p)
      p++;
    else
      p = name;
    _tcscpy(p, _T("security"));
    xn = xn.first_element_by_name(name);
    if (!xn.empty()) {
      _tcscpy(p, _T("requestedPrivileges"));
      xn = xn.first_element_by_name(name);
      if (!xn.empty()) {
        _tcscpy(p, _T("requestedExecutionLevel"));
        xml_node x1 = xn.first_element_by_attribute(name, _T("level"),
                                                    _T("requireAdministrator"));
        if (!x1.empty())
          bReqAdmin = TRUE;
        else {
          x1 = xn.first_element_by_name(name);
          if (!x1.empty())
            bReqAdmin = 2;
        }
      }
    }
  }
  return bReqAdmin;
}

BOOL CALLBACK EnumResProc(HMODULE hExe, LPCTSTR rType, LPTSTR rName,
                          LONG_PTR lParam) {
  UNREFERENCED_PARAMETER(rType);
  BOOL bAbort = FALSE;
  *((BOOL *)lParam) = FALSE;
  HRSRC hr = FindResource(hExe, rName, RT_MANIFEST);
  DWORD siz = SizeofResource(hExe, hr);
  HGLOBAL hg = LoadResource(hExe, hr);
  LPTSTR Manifest = (LPTSTR)calloc(siz + 1, 2);
  memmove(Manifest, LockResource(hg), siz);
  UnlockResource(hg);
  FreeResource(hg);
#ifdef UNICODE
  if ((Manifest[0] != 0xFFFE) && (Manifest[0] != 0xFEFF)) {
    LPTSTR m = (LPTSTR)calloc(siz + 1, 2);
    memmove(m, Manifest, 2 * siz);
    MultiByteToWideChar(CP_UTF8, 0, (char *)m, siz, Manifest, siz);
    free(m);
    InfoDBGTrace("RequiresAdmin: MultiByteToWideChar!");
  }
#else //UNICODE
  if ((Manifest[0] == 0xFF) || (Manifest[0] == 0xFE)) {
    LPTSTR m = (LPTSTR)calloc(siz + 1, 2);
    memmove(m, Manifest, 2 * siz);
    WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)m, siz / 2, Manifest, siz / 2, 0,
                        0);
    free(m);
    InfoDBGTrace("RequiresAdmin: WideCharToMultiByte!");
  }
#endif //UNICODE
  if (Manifest) {
    xml_parser xml; // Construct.
    if (xml.parse(Manifest))
      bAbort = RequiresAdmin(xml.document());
  }
  free(Manifest);
  if (bAbort)
    *((BOOL *)lParam) = bAbort;
  return bAbort != 1;
}

BOOL RequiresAdmin(LPCTSTR FileName) {
  if (!GetTestReqAdmin)
    return FALSE;
  TCHAR FName[4096];
  _tcscpy(FName, FileName);
  PathRemoveArgs(FName);
  PathUnquoteSpaces(FName);
  InfoDBGTrace1("RequiresAdmin(%s)?", FName);
  BOOL bReqAdmin = FALSE;
  HINSTANCE hExe = LoadLibraryEx(FName, 0, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
  if (hExe) {
    InfoDBGTrace1("RequiresAdmin(%s) LoadLib ok", FName);
    bReqAdmin = -1;
    EnumResourceNames(hExe, RT_MANIFEST, EnumResProc, (LONG_PTR)&bReqAdmin);
    FreeLibrary(hExe);
    if (bReqAdmin == -1) {
      bReqAdmin = FALSE;
      InfoDBGTrace1("RequiresAdmin(%s) EnumResourceNames failed", FName);
    } else if (bReqAdmin == 2)
      // requestedExecutionLevel present, but Admin not required
      return FALSE;
    else if (!bReqAdmin) {
      TCHAR s[4096];
      _stprintf(s, _T("%s.%s"), FName, _T("manifest"));
      xml_parser xml;
      if (xml.parse_file(s)) {
        bReqAdmin = RequiresAdmin(xml.document());
        if (bReqAdmin == 2)
          // requestedExecutionLevel present, but Admin not required
          return FALSE;
        if (bReqAdmin)
          InfoDBGTrace1("RequiresAdmin(%s) external Manifest OK", FName);
      } else
        InfoDBGTrace1("RequiresAdmin(%s) no external Manifest!", FName);
    }
    // Try FileName Pattern matching
    if (!bReqAdmin) {
      InfoDBGTrace1("RequiresAdmin(%s) FileName match???", FName);
      // Split path parts
      TCHAR file[4096];
      TCHAR ext[4096];
      // Get File, Ext
      _tcscpy(file, FName);
      _tcsupr(file);
      _tcscpy(ext, PathFindExtension(file));
      PathRemoveExtension(file);
      PathStripPath(file);
      InfoDBGTrace1("RequiresAdmin(%s) Extension match???", ext);
      bReqAdmin =
          (_tcsicmp(ext, _T(".MSI")) == 0) || (_tcsicmp(ext, _T(".MSC")) == 0);
      if (bReqAdmin)
        InfoDBGTrace1("RequiresAdmin(%s) Extension match", ext);
      if (!bReqAdmin) {
        if ((_tcsicmp(ext, _T(".EXE")) == 0) ||
            (_tcsicmp(ext, _T(".CMD")) == 0) ||
            (_tcsicmp(ext, _T(".LNK")) == 0) ||
            (_tcsicmp(ext, _T(".COM")) == 0) ||
            (_tcsicmp(ext, _T(".PIF")) == 0) ||
            (_tcsicmp(ext, _T(".BAT")) == 0)) {
          InfoDBGTrace1("RequiresAdmin(%s) Executable Extension!", FName);
          bReqAdmin = (_tcsstr(file, _T("INSTALL")) != 0) ||
                      (_tcsstr(file, _T("SETUP")) != 0) ||
                      (_tcsstr(file, _T("UPDATE")) != 0) ||
                      (_tcsstr(file, _T("UPGRADE")) != 0);
          if (bReqAdmin)
            InfoDBGTrace1("RequiresAdmin(%s) FileName match", file);
        }
      }
    }
  } else
    InfoDBGTrace1("RequiresAdmin(%s) LoadLibraryEx failed", FName);
  InfoDBGTrace2("RequiresAdmin(%s)==%d", FName, bReqAdmin);
  return bReqAdmin;
}
