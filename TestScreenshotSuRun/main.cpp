#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
extern "C" {
#include <windows.h>
}
#include "header.h"
#pragma comment(lib, "gdi32.lib")


int main() {
  // Initialize and capture the screen
  Test::CBlurredScreen1 screenCapture;
  screenCapture.Init();

  // Optionally show the captured screen (for debugging)
  screenCapture.Show(false); // false for no fade-in

  // Save the captured bitmap to a file (for inspection)
  HBITMAP hBitmap = screenCapture.m_bm; // Access the captured bitmap
  if (hBitmap) {
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);

    HDC hdc = CreateCompatibleDC(NULL);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize =
        ((bm.bmWidth * bi.biBitCount + 31) / 32) * 4 * bm.bmHeight;

    bmfHeader.bfType = 0x4D42; // "BM"
    bmfHeader.bfSize =
        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BYTE *lpbitmap = new BYTE[dwBmpSize];
    GetDIBits(hdc, hBitmap, 0, bm.bmHeight, lpbitmap, (BITMAPINFO *)&bi,
              DIB_RGB_COLORS);

    SelectObject(hdc, hOldBitmap);
    DeleteDC(hdc);

    FILE *fp = NULL; // Initialize to NULL
    errno_t err = fopen_s(&fp, "captured_screen.bmp", "wb");
    if (fp) {
      fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, fp);
      fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, fp);
      fwrite(lpbitmap, dwBmpSize, 1, fp);
      fclose(fp);
      std::cout << "Captured screen saved to captured_screen.bmp" << std::endl;
    } else {
      std::cerr << "Error saving bitmap!" << std::endl;
    }

    delete[] lpbitmap;
  }

  // Clean up
  screenCapture.Done();

  // Keep the console window open
  std::cout << "Press any key to exit..." << std::endl;
  std::cin.get();

  return 0;
}