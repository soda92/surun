// clang-format off
//go:build ignore
// clang-format on
#pragma once
#include <windows.h>

#define VERSION_HH 1
#define VERSION_HL 2
#define VERSION_LH 1
#define VERSION_LL 6
// #define VERSION_BETA 1

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#if (VERSION_BETA != 0)
#define LPCTSTR_VERSION                                                           \
  _T(STRINGIZE(VERSION_HH))                                                       \
  _T(".") _T(STRINGIZE(VERSION_HL)) _T(".") _T(STRINGIZE(VERSION_LH)) _T(".") _T( \
      STRINGIZE(VERSION_LL)) _T("b") _T(STRINGIZE(VERSION_BUILD))
// VER_INTERNALNAME will be used to rename InstallSuRun.exe
#define VER_INTERNALNAME                                                       \
  "InstallSuRun" STRINGIZE(VERSION_HH) STRINGIZE(VERSION_HL)                               \
      STRINGIZE(VERSION_LH) STRINGIZE(VERSION_LL) "b" STRINGIZE(VERSION_BETA)
#else //(VERSION_BETA != 0)
#define LPCTSTR_VERSION                                                        \
  _T(STRINGIZE(VERSION_HH))                                                    \
  _T(".") _T(STRINGIZE(VERSION_HL)) _T(".") _T(                                \
      STRINGIZE(VERSION_LH)) _T(".") _T(STRINGIZE(VERSION_LL))
// VER_INTERNALNAME will be used to rename InstallSuRun.exe
#define VER_INTERNALNAME                                                       \
  "InstallSuRun" STRINGIZE(VERSION_HH) STRINGIZE(VERSION_HL)                               \
      STRINGIZE(VERSION_LH) STRINGIZE(VERSION_LL)
#endif //(VERSION_BETA != 0)

#define VER_COMPANYNAME "http://kay-bruns.de"
#define VER_LEGALCOPYRIGHT "Copyright (C) 2007-2023"
#define VER_ORIGINALFILENAME
#define VER_PRODUCTNAME "SuperUserRun"

#define VER_FILE_VERSION VERSION_HH, VERSION_HL, VERSION_LH, VERSION_LL
#define VER_FILE_VERSION_STR                                                   \
  STRINGIZE(VERSION_HH) "," STRINGIZE(VERSION_HL) "," STRINGIZE(VERSION_LH) "," STRINGIZE(VERSION_LL)
#define VER_PRODUCT_VERSION VERSION_HH, VERSION_HL, VERSION_LH, VERSION_LL
#define VER_PRODUCT_VERSION_STR                                                \
  STRINGIZE(VERSION_HH) "," STRINGIZE(VERSION_HL) "," STRINGIZE(VERSION_LH) "," STRINGIZE(VERSION_LL)
