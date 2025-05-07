#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
extern "C" {
#include <windows.h>
#include <gdiplus.h>
}

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;
std::wstring GetTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm;
  localtime_s(&now_tm, &now_c);
  wchar_t buffer[64];
  std::wcsftime(buffer, sizeof(buffer), L"%Y%m%d_%H%M%S", &now_tm);
  return std::wstring(buffer);
}

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
  UINT numEncoders;
  UINT size;
  ImageCodecInfo *pImageCodecInfo = nullptr;

  GetImageEncodersSize(&numEncoders, &size);
  if (size == 0) {
    return -1; // Failure
  }

  pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
  if (pImageCodecInfo == nullptr) {
    return -1; // Failure
  }

  GetImageEncoders(numEncoders, size, pImageCodecInfo);

  for (UINT j = 0; j < numEncoders; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j; // Success
    }
  }

  free(pImageCodecInfo);
  return -1; // Failure
}

int SaveBitmapToFile(HBITMAP hBitmap, const std::wstring &filename) {
  Bitmap bitmap(hBitmap, nullptr);
  CLSID encoderClsid;
  // Get the CLSID for the BMP encoder
  INT result = GetEncoderClsid(L"image/bmp", &encoderClsid);
  if (result != Ok) {
    std::wcerr << L"Error getting BMP encoder CLSID: " << result << std::endl;
    return 1;
  }
  result = bitmap.Save(filename.c_str(), &encoderClsid, nullptr);
  if (result != Ok) {
    std::wcerr << L"Error saving bitmap: " << result << std::endl;
    return 1;
  }
  std::wcout << L"Screenshot saved to: " << filename << std::endl;
  return 0;
}

int main() {
  // Initialize GDI+
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

  DPI_AWARENESS_CONTEXT oldContext =
      SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

  HDC hScreenDC = GetDC(nullptr); // Get DC for the entire virtual screen
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
  HBITMAP hBitmap =
      CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

  // Copy screen to memory DC
  BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, screenLeft,
         screenTop, SRCCOPY);

  SelectObject(hMemoryDC, hOldBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(nullptr, hScreenDC);

  // Save the bitmap to a file
  std::wstring timestamp = GetTimestamp();
  std::wstring filename = L"screenshot.bmp";
  SaveBitmapToFile(hBitmap, filename);

  DeleteObject(hBitmap);

  // Shutdown GDI+
  GdiplusShutdown(gdiplusToken);
  SetThreadDpiAwarenessContext(oldContext); // Restore previous context

  return 0;
}