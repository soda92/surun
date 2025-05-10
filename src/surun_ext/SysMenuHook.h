#include <windows.h>
extern "C" {
__declspec(dllexport) BOOL InstallSysMenuHook();
__declspec(dllexport) BOOL UninstallSysMenuHook();
}
