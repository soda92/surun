// clang-format off
//go:build ignore
// clang-format on
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

//////////////////////////////////////////////////////////////////////////////
// Home of CBlurredScreen
//
// This class captures the Windows Desktop (containing all windows)
// into a bitmap, blurres and darkens that bitmap and displays a fullscreen
// window with that bitmap as client area
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <algorithm>
#include "Helpers.h"
#include "DbgTrace.h"
extern "C" {
    #include <gdiplus.h>
    #include <TCHAR.h>
    #include <WINDOWS.h>
}
#pragma comment(lib, "Gdiplus.lib")
using std::min;
using std::max;
// Simplified 3x3 Gausian blur
inline void Blur(COLORREF *pDst, COLORREF *pSrc, DWORD w, DWORD h) {
  CTimeLog l(_T("Blur %dx%d"), w, h);
  DWORD x, y;
  DWORD c, c1, c2;
  COLORREF *p;
  COLORREF *d = pDst + w + 1;
  for (y = 1; y < h - 1; y++, d += 2)
    for (x = 1; x < w - 1; x++) {
      p = pSrc + x - 1 + (y - 1) * w;
      c = *p++;
      c2 = c & 0x0000FF00;
      c1 = c & 0x00FF00FF;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = *p;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      p += w - 2;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = (*p++) << 2;
      c2 += c & 0x0003FC00;
      c1 += c & 0x03FC03FC;
      c = (*p) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      p += w - 2;
      c = *p++;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = *p;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      *d++ = ((c2 >> 5) & 0x0000FF00) + ((c1 >> 5) & 0x00FF00FF);
    }
}

// Simplified 3x3 Gausian blur
inline void BlurBright(COLORREF *pDst, COLORREF *pSrc, DWORD w, DWORD h) {
  // CTimeLog l(_T("Blur %dx%d"),w,h);
  DWORD x, y;
  DWORD c, c1, c2;
  COLORREF *p;
  COLORREF *d = pDst + w + 1;
  for (y = 1; y < h - 1; y++, d += 2)
    for (x = 1; x < w - 1; x++) {
      p = pSrc + x - 1 + (y - 1) * w;
      c = *p++;
      c2 = c & 0x0000FF00;
      c1 = c & 0x00FF00FF;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = *p;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      p += w - 2;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = (*p++) << 2;
      c2 += c & 0x0003FC00;
      c1 += c & 0x03FC03FC;
      c = (*p) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      p += w - 2;
      c = *p++;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      c = (*p++) << 1;
      c2 += c & 0x0001FE00;
      c1 += c & 0x01FE01FE;
      c = *p;
      c2 += c & 0x0000FF00;
      c1 += c & 0x00FF00FF;
      *d++ = 0xC0C0C0C0 + ((c2 >> 6) & 0x0000FF00) + ((c1 >> 6) & 0x00FF00FF);
    }
}

inline HBITMAP CreateDIB(HDC dc, int w, int h, void **Bits) {
  BITMAPINFO bmi = {{sizeof(BITMAPINFOHEADER), 0, 0, 1, 32, 0, 0, 0, 0}, 0};
  bmi.bmiHeader.biHeight = h;
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biSizeImage = w * h * 4;
  return CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, Bits, NULL, 0);
}

class CBlurredScreen {
public:
  CBlurredScreen() {
    m_hWnd = 0;
    m_hWndTrans = 0;
    m_x = 0;
    m_y = 0;
    m_dx = 0;
    m_dy = 0;
    m_pbmBits = 0;
    m_bm = 0;
    m_pblurbmBits = 0;
    m_blurbm = 0;
    m_Thread = NULL;
    timeBeginPeriod(1);
  }
  ~CBlurredScreen() {
    Done();
    timeEndPeriod(1);
  }
  HWND hWnd() { return m_hWndTrans; };
  void Init() {
    DPI_AWARENESS_CONTEXT oldContext =
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    m_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_dx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_dy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    HDC dc = GetDC(0);
    HDC MemDC = CreateCompatibleDC(dc);
    m_bm = CreateDIB(dc, m_dx, m_dy, &m_pbmBits);
    (HBITMAP) SelectObject(MemDC, m_bm);
    BitBlt(MemDC, 0, 0, m_dx, m_dy, dc, m_x, m_y, SRCCOPY | CAPTUREBLT);
    DeleteDC(MemDC);
    ReleaseDC(0, dc);
    GdiFlush();
  }
  void Done() {
    if (m_Thread) {
      if (WaitForSingleObject(m_Thread, 2000) != WAIT_OBJECT_0)
        TerminateThread(m_Thread, 0);
      CloseHandle(m_Thread);
    }
    m_Thread = NULL;
    if (m_hWnd) {
      SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);
      DestroyWindow(m_hWnd);
    }
    m_hWnd = 0;
    if (m_hWndTrans) {
      SetWindowLongPtr(m_hWndTrans, GWLP_USERDATA, 0);
      DestroyWindow(m_hWndTrans);
    }
    m_hWndTrans = 0;
    if (m_bm)
      DeleteObject(m_bm);
    m_bm = 0;
    if (m_blurbm)
      DeleteObject(m_blurbm);
    m_blurbm = 0;
    UnregisterClass(_T("ScreenWndClass"), GetModuleHandle(0));
  }
  void Show(bool bFadeIn) {
    WNDCLASS wc = {0};
    OSVERSIONINFO oie;
    oie.dwOSVersionInfoSize = sizeof(oie);
    GetVersionEx(&oie);
    bool bWin2k = (oie.dwMajorVersion == 5) && (oie.dwMinorVersion == 0);
    wc.lpfnWndProc = CBlurredScreen::WindowProc;
    wc.lpszClassName = _T("ScreenWndClass");
    wc.hInstance = GetModuleHandle(0);
    RegisterClass(&wc);
    m_hWnd = CreateWindowEx(WS_EX_NOACTIVATE, wc.lpszClassName, _T("ScreenWnd"),
                            WS_VISIBLE | WS_POPUP | WS_DISABLED | WS_VISIBLE,
                            m_x, m_y, m_dx, m_dy, 0, 0, wc.hInstance, 0);
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
    RedrawWindow(m_hWnd, 0, 0, RDW_INTERNALPAINT | RDW_UPDATENOW);
    if ((!bWin2k) && bFadeIn) {
      MsgLoop();
      m_hWndTrans =
          CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_LAYERED, wc.lpszClassName,
                         _T("ScreenWnd"), WS_POPUP | WS_DISABLED | WS_VISIBLE,
                         m_x, m_y, m_dx, m_dy, m_hWnd, 0, wc.hInstance, 0);
      SetWindowLongPtr(m_hWndTrans, GWLP_USERDATA, (LONG_PTR)this);
      SetWindowPos(m_hWndTrans, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
    } else {
      m_hWndTrans = m_hWnd;
      m_hWnd = 0;
      if (m_bm) {
        HDC dc = GetDC(0);
        m_blurbm = CreateDIB(dc, m_dx, m_dy, &m_pblurbmBits);
        ReleaseDC(0, dc);
        GdiFlush();
        Blur((COLORREF *)m_pblurbmBits, (COLORREF *)m_pbmBits, m_dx, m_dy);
        DeleteObject(m_bm);
        InvalidateRect(m_hWndTrans, 0, true);
      }
      m_pbmBits = 0;
      m_bm = 0;
    }
    MsgLoop();
  }
  static void MsgLoop() {
    MSG msg;
    int count = 0;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && (count++ < 100)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  void FadeIn() {
    m_hDesk = GetThreadDesktop(GetCurrentThreadId());
    if (m_hWndTrans && m_hWnd && m_bm && (m_Thread == NULL))
      m_Thread = CreateThread(0, 0, BlurProc, this, 0, 0);
  }
  void FadeOut() {
    if (m_Thread && m_hWndTrans && m_hWnd) {
      if (WaitForSingleObject(m_Thread, 2000) != WAIT_OBJECT_0)
        TerminateThread(m_Thread, 0);
      DWORD StartTime = timeGetTime();
      BYTE a = 255;
      while (a) {
        a = 255 - (BYTE)min(255,(int)( (timeGetTime() - StartTime) / 2));
        SetLayeredWindowAttributes(m_hWndTrans, 0, a, LWA_ALPHA);
        MsgLoop();
      }
    }
  }

public:
  static DWORD WINAPI BlurProc(void *p) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
    Sleep(200);
    CBlurredScreen *bs = (CBlurredScreen *)p;
    SetThreadDesktop(bs->m_hDesk);
    HDC dc = GetDC(0);
    bs->m_blurbm = CreateDIB(dc, bs->m_dx, bs->m_dy, &bs->m_pblurbmBits);
    ReleaseDC(0, dc);
    GdiFlush();
    Blur((COLORREF *)bs->m_pblurbmBits, (COLORREF *)bs->m_pbmBits, bs->m_dx,
         bs->m_dy);
    SetLayeredWindowAttributes(bs->m_hWndTrans, 0, 0, LWA_ALPHA);
    RedrawWindow(bs->m_hWndTrans, 0, 0, RDW_INTERNALPAINT | RDW_UPDATENOW);
    DWORD StartTime = timeGetTime();
    for (;;) {
      BYTE a = (BYTE)min(255, (int)((timeGetTime() - StartTime) / 2));
      SetLayeredWindowAttributes(bs->m_hWndTrans, 0, a, LWA_ALPHA);
      MsgLoop();
      if (a == 255)
        return 0;
    }
    // return 0;
  }
  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam) {
    CBlurredScreen *bs =
        (CBlurredScreen *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (bs)
      return bs->WndProc(hWnd, msg, wParam, lParam);
    return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      HBITMAP bm = (hwnd == m_hWnd) ? m_bm : m_blurbm;
      if (bm) {
        HDC MemDC = CreateCompatibleDC(hdc);
        SelectObject(MemDC, bm);
        BitBlt(hdc, 0, 0, m_dx, m_dy, MemDC, 0, 0, SRCCOPY);
        DeleteDC(MemDC);
      }
      EndPaint(hwnd, &ps);
      return 0;
    }
    case WM_SETCURSOR:
      SetCursor(LoadCursor(0, IDC_WAIT));
      return TRUE;
    case WM_MOUSEACTIVATE: {
      if (LOWORD(lParam) == HTCAPTION)
        return MA_NOACTIVATEANDEAT;
      if (LOWORD(lParam) == HTSYSMENU) {
        if (m_hWnd)
          SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0,
                       SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
        if (m_hWndTrans)
          SetWindowPos(m_hWndTrans, HWND_TOP, 0, 0, 0, 0,
                       SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
      }
      return MA_NOACTIVATE;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  HWND m_hWnd;
  HWND m_hWndTrans;
  int m_x;
  int m_y;
  int m_dx;
  int m_dy;
  void *m_pbmBits;
  HBITMAP m_bm;
  void *m_pblurbmBits;
  HBITMAP m_blurbm;
  HANDLE m_Thread;
  HDESK m_hDesk;
};
