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
#include <TCHAR.h>
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>

#include "DBGTrace.h"
#include <lmerr.h>

#pragma warning(disable : 4996)
#pragma warning(disable : 4473)

void GetErrorName(int ErrorCode, LPTSTR s) {
  HMODULE hModule = NULL;
  LPTSTR MessageBuffer = 0;
  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_IGNORE_INSERTS |
                        FORMAT_MESSAGE_FROM_SYSTEM;
  if (ErrorCode >= NERR_BASE && ErrorCode <= MAX_NERR) {
    hModule = LoadLibraryEx(_T("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule != NULL)
      dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }
  FormatMessage(dwFormatFlags, hModule, ErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&MessageBuffer, 0, NULL);
  if (hModule != NULL)
    FreeLibrary(hModule);
  _sntprintf(s, 1024, _T("%d(0x%08X): %s"), ErrorCode, ErrorCode,
             MessageBuffer);
  LocalFree(MessageBuffer);
}

LPCTSTR MsgName(UINT msg) {
  switch (msg) {
#define MSGNAME(m)                                                             \
  case m:                                                                      \
    return _T(#m);
    MSGNAME(WM_NULL)
    MSGNAME(WM_CREATE)
    MSGNAME(WM_DESTROY)
    MSGNAME(WM_MOVE)
    MSGNAME(WM_SIZE)
    MSGNAME(WM_ACTIVATE)
    MSGNAME(WM_SETFOCUS)
    MSGNAME(WM_KILLFOCUS)
    MSGNAME(WM_ENABLE)
    MSGNAME(WM_SETREDRAW)
    MSGNAME(WM_SETTEXT)
    MSGNAME(WM_GETTEXT)
    MSGNAME(WM_GETTEXTLENGTH)
    MSGNAME(WM_PAINT)
    MSGNAME(WM_CLOSE)
    MSGNAME(WM_QUERYENDSESSION)
    MSGNAME(WM_QUERYOPEN)
    MSGNAME(WM_ENDSESSION)
    MSGNAME(WM_QUIT)
    MSGNAME(WM_ERASEBKGND)
    MSGNAME(WM_SYSCOLORCHANGE)
    MSGNAME(WM_SHOWWINDOW)
    MSGNAME(WM_WININICHANGE)
    MSGNAME(WM_DEVMODECHANGE)
    MSGNAME(WM_ACTIVATEAPP)
    MSGNAME(WM_FONTCHANGE)
    MSGNAME(WM_TIMECHANGE)
    MSGNAME(WM_CANCELMODE)
    MSGNAME(WM_SETCURSOR)
    MSGNAME(WM_MOUSEACTIVATE)
    MSGNAME(WM_CHILDACTIVATE)
    MSGNAME(WM_QUEUESYNC)
    MSGNAME(WM_GETMINMAXINFO)
    MSGNAME(WM_PAINTICON)
    MSGNAME(WM_ICONERASEBKGND)
    MSGNAME(WM_NEXTDLGCTL)
    MSGNAME(WM_SPOOLERSTATUS)
    MSGNAME(WM_DRAWITEM)
    MSGNAME(WM_MEASUREITEM)
    MSGNAME(WM_DELETEITEM)
    MSGNAME(WM_VKEYTOITEM)
    MSGNAME(WM_CHARTOITEM)
    MSGNAME(WM_SETFONT)
    MSGNAME(WM_GETFONT)
    MSGNAME(WM_SETHOTKEY)
    MSGNAME(WM_GETHOTKEY)
    MSGNAME(WM_QUERYDRAGICON)
    MSGNAME(WM_COMPAREITEM)
    MSGNAME(WM_GETOBJECT)
    MSGNAME(WM_COMPACTING)
    MSGNAME(WM_COMMNOTIFY)
    MSGNAME(WM_WINDOWPOSCHANGING)
    MSGNAME(WM_WINDOWPOSCHANGED)
    MSGNAME(WM_POWER)
    MSGNAME(WM_COPYDATA)
    MSGNAME(WM_CANCELJOURNAL)
    MSGNAME(WM_NOTIFY)
    MSGNAME(WM_INPUTLANGCHANGEREQUEST)
    MSGNAME(WM_INPUTLANGCHANGE)
    MSGNAME(WM_TCARD)
    MSGNAME(WM_HELP)
    MSGNAME(WM_USERCHANGED)
    MSGNAME(WM_NOTIFYFORMAT)
    MSGNAME(WM_CONTEXTMENU)
    MSGNAME(WM_STYLECHANGING)
    MSGNAME(WM_STYLECHANGED)
    MSGNAME(WM_DISPLAYCHANGE)
    MSGNAME(WM_GETICON)
    MSGNAME(WM_SETICON)
    MSGNAME(WM_NCCREATE)
    MSGNAME(WM_NCDESTROY)
    MSGNAME(WM_NCCALCSIZE)
    MSGNAME(WM_NCHITTEST)
    MSGNAME(WM_NCPAINT)
    MSGNAME(WM_NCACTIVATE)
    MSGNAME(WM_GETDLGCODE)
    MSGNAME(WM_SYNCPAINT)
    MSGNAME(WM_NCMOUSEMOVE)
    MSGNAME(WM_NCLBUTTONDOWN)
    MSGNAME(WM_NCLBUTTONUP)
    MSGNAME(WM_NCLBUTTONDBLCLK)
    MSGNAME(WM_NCRBUTTONDOWN)
    MSGNAME(WM_NCRBUTTONUP)
    MSGNAME(WM_NCRBUTTONDBLCLK)
    MSGNAME(WM_NCMBUTTONDOWN)
    MSGNAME(WM_NCMBUTTONUP)
    MSGNAME(WM_NCMBUTTONDBLCLK)
    MSGNAME(WM_NCXBUTTONDOWN)
    MSGNAME(WM_NCXBUTTONUP)
    MSGNAME(WM_NCXBUTTONDBLCLK)
    // MSGNAME(WM_INPUT)
    MSGNAME(WM_KEYDOWN)
    MSGNAME(WM_KEYUP)
    MSGNAME(WM_CHAR)
    MSGNAME(WM_DEADCHAR)
    MSGNAME(WM_SYSKEYDOWN)
    MSGNAME(WM_SYSKEYUP)
    MSGNAME(WM_SYSCHAR)
    MSGNAME(WM_SYSDEADCHAR)
    MSGNAME(WM_IME_STARTCOMPOSITION)
    MSGNAME(WM_IME_ENDCOMPOSITION)
    MSGNAME(WM_IME_COMPOSITION)
    MSGNAME(WM_INITDIALOG)
    MSGNAME(WM_COMMAND)
    MSGNAME(WM_SYSCOMMAND)
    MSGNAME(WM_TIMER)
    MSGNAME(WM_HSCROLL)
    MSGNAME(WM_VSCROLL)
    MSGNAME(WM_INITMENU)
    MSGNAME(WM_INITMENUPOPUP)
    MSGNAME(WM_MENUSELECT)
    MSGNAME(WM_MENUCHAR)
    MSGNAME(WM_ENTERIDLE)
    MSGNAME(WM_MENURBUTTONUP)
    MSGNAME(WM_MENUDRAG)
    MSGNAME(WM_MENUGETOBJECT)
    MSGNAME(WM_UNINITMENUPOPUP)
    MSGNAME(WM_MENUCOMMAND)
    MSGNAME(WM_CHANGEUISTATE)
    MSGNAME(WM_UPDATEUISTATE)
    MSGNAME(WM_QUERYUISTATE)
    MSGNAME(WM_CTLCOLORMSGBOX)
    MSGNAME(WM_CTLCOLOREDIT)
    MSGNAME(WM_CTLCOLORLISTBOX)
    MSGNAME(WM_CTLCOLORBTN)
    MSGNAME(WM_CTLCOLORDLG)
    MSGNAME(WM_CTLCOLORSCROLLBAR)
    MSGNAME(WM_CTLCOLORSTATIC)
    MSGNAME(WM_MOUSEMOVE)
    MSGNAME(WM_LBUTTONDOWN)
    MSGNAME(WM_LBUTTONUP)
    MSGNAME(WM_LBUTTONDBLCLK)
    MSGNAME(WM_RBUTTONDOWN)
    MSGNAME(WM_RBUTTONUP)
    MSGNAME(WM_RBUTTONDBLCLK)
    MSGNAME(WM_MBUTTONDOWN)
    MSGNAME(WM_MBUTTONUP)
    MSGNAME(WM_MBUTTONDBLCLK)
    MSGNAME(WM_MOUSEWHEEL)
    MSGNAME(WM_XBUTTONDOWN)
    MSGNAME(WM_XBUTTONUP)
    MSGNAME(WM_XBUTTONDBLCLK)
    MSGNAME(WM_PARENTNOTIFY)
    MSGNAME(WM_ENTERMENULOOP)
    MSGNAME(WM_EXITMENULOOP)
    MSGNAME(WM_NEXTMENU)
    MSGNAME(WM_SIZING)
    MSGNAME(WM_CAPTURECHANGED)
    MSGNAME(WM_MOVING)
    MSGNAME(WM_POWERBROADCAST)
    MSGNAME(WM_DEVICECHANGE)
    MSGNAME(WM_MDICREATE)
    MSGNAME(WM_MDIDESTROY)
    MSGNAME(WM_MDIACTIVATE)
    MSGNAME(WM_MDIRESTORE)
    MSGNAME(WM_MDINEXT)
    MSGNAME(WM_MDIMAXIMIZE)
    MSGNAME(WM_MDITILE)
    MSGNAME(WM_MDICASCADE)
    MSGNAME(WM_MDIICONARRANGE)
    MSGNAME(WM_MDIGETACTIVE)
    MSGNAME(WM_MDISETMENU)
    MSGNAME(WM_ENTERSIZEMOVE)
    MSGNAME(WM_EXITSIZEMOVE)
    MSGNAME(WM_DROPFILES)
    MSGNAME(WM_MDIREFRESHMENU)
    MSGNAME(WM_IME_SETCONTEXT)
    MSGNAME(WM_IME_NOTIFY)
    MSGNAME(WM_IME_CONTROL)
    MSGNAME(WM_IME_COMPOSITIONFULL)
    MSGNAME(WM_IME_SELECT)
    MSGNAME(WM_IME_CHAR)
    MSGNAME(WM_IME_REQUEST)
    MSGNAME(WM_IME_KEYDOWN)
    MSGNAME(WM_IME_KEYUP)
    MSGNAME(WM_MOUSEHOVER)
    MSGNAME(WM_MOUSELEAVE)
    MSGNAME(WM_NCMOUSEHOVER)
    MSGNAME(WM_NCMOUSELEAVE)
    // MSGNAME(WM_WTSSESSION_CHANGE)
    MSGNAME(WM_CUT)
    MSGNAME(WM_COPY)
    MSGNAME(WM_PASTE)
    MSGNAME(WM_CLEAR)
    MSGNAME(WM_UNDO)
    MSGNAME(WM_RENDERFORMAT)
    MSGNAME(WM_RENDERALLFORMATS)
    MSGNAME(WM_DESTROYCLIPBOARD)
    MSGNAME(WM_DRAWCLIPBOARD)
    MSGNAME(WM_PAINTCLIPBOARD)
    MSGNAME(WM_VSCROLLCLIPBOARD)
    MSGNAME(WM_SIZECLIPBOARD)
    MSGNAME(WM_ASKCBFORMATNAME)
    MSGNAME(WM_CHANGECBCHAIN)
    MSGNAME(WM_HSCROLLCLIPBOARD)
    MSGNAME(WM_QUERYNEWPALETTE)
    MSGNAME(WM_PALETTEISCHANGING)
    MSGNAME(WM_PALETTECHANGED)
    MSGNAME(WM_HOTKEY)
    MSGNAME(WM_PRINT)
    MSGNAME(WM_PRINTCLIENT)
    MSGNAME(WM_APPCOMMAND)
// MSGNAME(WM_THEMECHANGED)
#undef MSGNAME
  }
  static TCHAR s[10];
  _stprintf(s, _T("0x%08X"), msg);
  return s;
}

static TCHAR err[4096];
LPCTSTR GetErrorNameStatic(int ErrorCode) {
  GetErrorName(ErrorCode, err);
  return err;
}

LPCTSTR GetLastErrorNameStatic() { return GetErrorNameStatic(GetLastError()); }

void WriteLogA(LPSTR S, ...) {
  char tmp[MAX_PATH];
  GetTempPathA(MAX_PATH, tmp);
  strcat(tmp, "SuRunLog.log");
  FILE *f = fopen(tmp, "a+t");
  if (f) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "%02d:%02d:%02d.%03d [%d]: %s", st.wHour, st.wMinute, st.wSecond,
            st.wMilliseconds, GetCurrentProcessId());
    va_list va;
    va_start(va, S);
    vfprintf(f, S, va);
    va_end(va);
    fclose(f);
  }
}

void WriteLog(LPTSTR S, ...) {
#ifdef UNICODE
  WCHAR tmp[MAX_PATH];
  GetTempPathW(MAX_PATH, tmp);
  wcscat(tmp, L"SuRunLog.log");
  FILE *f = _wfopen(tmp, L"a+t");
  if (f) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(f, L"%02d:%02d:%02d.%03d [%d]: ", st.wHour, st.wMinute, st.wSecond,
             st.wMilliseconds, GetCurrentProcessId());
    va_list va;
    va_start(va, S);
    vfwprintf(f, S, va);
    va_end(va);
    fclose(f);
  }
#else  // UNICODE
  WriteLogA(S);
#endif // UNICODE
}

// #pragma pack(push,1)
// typedef struct
//{
//   DWORD PID;
//   char s[4096-sizeof(DWORD)];
// }DBGSTRBUF;
// #pragma pack(pop)
////DbgOutA was created after reading:
////http://unixwiz.net/techtips/outputdebugstring.html
// void DbgOutA(LPCSTR s)
//{
//   static HANDLE DBWMtx=NULL;
//   if (!DBWMtx)
//     DBWMtx=OpenMutexA(SYNCHRONIZE,false,"DBWinMutex");
//   if (!DBWMtx)
//   {
//     //Just in case Microsoft changes OutputDebugString in future:
//     OutputDebugStringA("FATAL: DBWinMutex not available\n");
//     return;
//   }
//   WaitForSingleObject(DBWMtx,INFINITE);
//   HANDLE
//   DBWBuf=OpenFileMappingA(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,"DBWIN_BUFFER");
//   if (!DBWBuf)
//   {
//     ReleaseMutex(DBWMtx);
//     //Just in case Microsoft changes OutputDebugString in future:
//     OutputDebugStringA("FATAL: DBWIN_BUFFER not available\n");
//     return;
//   }
//   DBGSTRBUF* Buf=(DBGSTRBUF*)MapViewOfFile(DBWBuf,FILE_MAP_WRITE,0,0,0);
//   HANDLE evRdy=OpenEventA(EVENT_MODIFY_STATE,FALSE,"DBWIN_DATA_READY");
//   HANDLE evAck=OpenEventA(SYNCHRONIZE,FALSE,"DBWIN_BUFFER_READY");
//   try
//   {
//     if (evRdy && WaitForSingleObject(evAck,5000) == WAIT_OBJECT_0)
//     {
//       Buf->PID=GetCurrentProcessId();
//       memset(&Buf->s,0,sizeof(Buf->s));
//       memmove(&Buf->s,s,min(strlen(s),sizeof(Buf->s)-1));
//       SetEvent(evRdy);
//     }else
//     {
//       //Just in case Microsoft changes OutputDebugString in future:
//       OutputDebugStringA("FATAL: DBWIN_xxx events not available\n");
//     }
//   }
//  }__except((GetExceptionCode()!=DBG_PRINTEXCEPTION_C)?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH)
//   {
//     //Just in case Microsoft changes OutputDebugString in future:
//     OutputDebugStringA("FATAL: Exception in DbgOutA\n");
//   }
//   if (evAck)
//     CloseHandle(evAck);
//   if (evRdy)
//     CloseHandle(evRdy);
//   if (Buf)
//     UnmapViewOfFile(Buf);
//   CloseHandle(DBWBuf);
//   ReleaseMutex(DBWMtx);
// }

void TRACEx(LPCTSTR s, ...) {
  try {
    static TCHAR S[4096] = {0};
    va_list va;
    va_start(va, s);
    _vsntprintf(S, 4095, s, va);
    va_end(va);
    LPTSTR c0 = _tcschr(S, ':');
    LPTSTR c1 = _tcschr(S, '\\');
    LPTSTR c2 = _tcschr(S, '.');
    LPTSTR c3 = _tcschr(S, '(');
    LPTSTR c4 = _tcschr(S, ')');
    if ((c0 < c1) && (c1 < c2) && (c2 < c3) && (c3 < c4)) {
      *c2 = 0;
      c0 = _tcsrchr(c0, '\\') + 1;
      *c2 = '.';
      memmove(S, c0, (_tcslen(c0) + 1) * sizeof(TCHAR));
    }
    OutputDebugString(S);
    //    char Sa[4096]={0};
    //	  WideCharToMultiByte(CP_ACP,0,S,_tcslen(S),Sa,4096,NULL,NULL);
    //	  DbgOutA(Sa);
    WriteLog(S);
  } catch(...) {
    OutputDebugStringA("FATAL: Exception in TRACEx");
  }
}

void TRACExA(LPCSTR s, ...) {
  try {
    static char S[4096] = {0};
    va_list va;
    va_start(va, s);
    _vsnprintf(S, 4095, s, va);
    va_end(va);
    LPSTR c0 = strchr(S, ':');
    LPSTR c1 = strchr(S, '\\');
    LPSTR c2 = strchr(S, '.');
    LPSTR c3 = strchr(S, '(');
    LPSTR c4 = strchr(S, ')');
    if ((c0 < c1) && (c1 < c2) && (c2 < c3) && (c3 < c4)) {
      *c2 = 0;
      c0 = strrchr(c0, '\\') + 1;
      *c2 = '.';
      memmove(S, c0, strlen(c0) + 1);
    }
    //    DbgOutA(S);
    OutputDebugStringA(S);
    WriteLogA(S);
  } catch (...) {
    OutputDebugStringA("FATAL: Exception in TRACExA");
  }
}
