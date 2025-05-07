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

#pragma once
#include <shlobj.h>

extern "C" {
__declspec(dllexport) void RemoveShellExt();
__declspec(dllexport) void InstallShellExt();
__declspec(dllexport) void TerminateAllSuRunnedProcesses(HANDLE hToken);
};

// {2C7B6088-5A77-4d48-BE43-30337DCA9A86}
DEFINE_GUID(CLSID_ShellExtension, 0x2c7b6088, 0x5a77, 0x4d48, 0xbe, 0x43, 0x30,
            0x33, 0x7d, 0xca, 0x9a, 0x86);
#define sGUID L"{2C7B6088-5A77-4d48-BE43-30337DCA9A86}"
