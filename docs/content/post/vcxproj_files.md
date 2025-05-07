---
date: 2025-05-01T01:08:21+08:00
title: Vcxproj_Files
---

# `.vcxproj` in Visual Studio

https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure?view=msvc-170

If you intend to maintain your project properties in the IDE, we recommend you only create and modify your .vcxproj projects in the IDE, and avoid manual edits to the files. In most cases, you never need to manually edit the project file. Manual edits may break the project connections required to modify project settings in the Visual Studio property pages, and can cause build errors that are difficult to debug and repair. For more information about using the property pages, see Set C++ compiler and build properties in Visual Studio.


```python
from surun_tools.glob1 import glob_files
for i in glob_files("*.vcxproj"):
    print(i)
```

    C:\src\surun\install_surun\InstallSuRunVC9.vcxproj
    C:\src\surun\PC\SuRunC.vcxproj
    C:\src\surun\PC\SuRunExtVC9.vcxproj
    C:\src\surun\PC\SuRunVC9.vcxproj
    C:\src\surun\tests\TestScreenshot\TestScreenshot.vcxproj
    C:\src\surun\tests\TestScreenshotSuRun\TestScreenshotSuRun.vcxproj
    

## include files


```python
for i in glob_files("*.vcxproj"):
    k = i.read_text(encoding="utf8")
    import re

    result = re.findall(r"Include=\"(.*)\"", k)
    result = list(filter(lambda x: "|" not in x, result))
    print(i, result)
```

    C:\src\surun\install_surun\InstallSuRunVC9.vcxproj ['InstallSuRun.cpp', 'InstallSuRun.rc', 'resource.h', 'app.manifest', '..\\ReleaseUx64\\SuRun.exe', '..\\ReleaseUx64\\SuRun32.bin', '..\\ReleaseUx64\\SuRunExt.dll', '..\\ReleaseUx64\\SuRunExt32.dll', '..\\ReleaseU\\SuRun.exe', '..\\ReleaseU\\SuRunExt.dll', 'InstSuRunVer.rc2', '..\\res\\SuRun.ico', '..\\SuRunExt\\SuRunExtVC9.vcxproj', '..\\SuRunVC9.vcxproj']
    C:\src\surun\PC\SuRunC.vcxproj ['SuRunC.cpp', 'SuRunC.rc', 'resource.h', 'res\\SuRun.ico', 'res\\app.manifest']
    C:\src\surun\PC\SuRunExtVC9.vcxproj ['..\\DBGTrace.cpp', '..\\DynWTSAPI.cpp', '..\\Helpers.cpp', '..\\IsAdmin.cpp', '..\\LogonDlg.cpp', '..\\LSALogon.cpp', '..\\lsa_laar.cpp', '..\\Setup.cpp', '..\\sspi_auth.cpp', '..\\UserGroups.cpp', '..\\WinStaDesk.cpp', 'IATHook.cpp', 'SuRunext.cpp', 'SysMenuHook.cpp', 'SuRunExt.Def', 'SuRunExt32.Def', 'SuRunExtVer.rc2', 'SuRunext.rc', '..\\DynWTSAPI.h', '..\\Helpers.h', 'IATHook.h', 'resource.h', 'SuRunext.h', 'SysMenuHook.h', '..\\res\\app.manifest', '..\\res\\icon1.ico', '..\\res\\Shield.ico', '..\\res\\SuRun.ico']
    C:\src\surun\PC\SuRunVC9.vcxproj ['DBGTrace.cpp', 'DynWTSAPI.cpp', 'Helpers.cpp', 'IsAdmin.cpp', 'LogonDlg.cpp', 'LSALogon.cpp', 'lsa_laar.cpp', 'main.cpp', 'ReqAdmin.cpp', 'Service.cpp', 'Setup.cpp', 'sspi_auth.cpp', 'TrayMsgWnd.cpp', 'TrayShowAdmin.cpp', 'UserGroups.cpp', 'WatchDog.cpp', 'WinStaDesk.cpp', 'launcher.cpp', 'SuRun.rc', 'anchor.h', 'main.h', 'CmdLine.h', 'DBGTrace.H', 'DynWTSAPI.h', 'Helpers.h', 'IsAdmin.h', 'LogonDlg.h', 'LSALogon.h', 'lsa_laar.h', 'pugxml.h', 'ReqAdmin.h', 'resource.h', 'ResStr.h', 'ScreenSnap.h', 'Service.h', 'Setup.h', 'sspi_auth.h', 'SuRunVer.h', 'TrayMsgWnd.h', 'TrayShowAdmin.h', 'UserGroups.h', 'WatchDog.h', 'WinStaDesk.h', 'res\\Admin.ico', 'res\\AutoCancel.ico', 'res\\CancelWindows.ico', 'res\\ico10605.ico', 'res\\ico10606.ico', 'res\\ico10607.ico', 'res\\neverquestion.ico', 'res\\NoAdmin.ico', 'res\\NoQuestion.ico', 'res\\NoRestrict.ico', 'res\\NoWindow.ico', 'res\\NoWindows.ico', 'res\\Question.ico', 'res\\Restrict.ico', 'res\\SHADMIN.ico', 'res\\Shield.ico', 'res\\SrAdmin.ico', 'res\\SuRun.ico', 'res\\SuRunBW.ico', 'res\\Windows.ico', 'res\\app.manifest', 'SuRunVer.rc2', 'ChangeLog.txt', 'gpedit.txt', 'ReadMe.txt', 'SuRunExt\\SuRunExtVC9.vcxproj']
    C:\src\surun\tests\TestScreenshot\TestScreenshot.vcxproj ['TestScreenshot.cpp', 'app.manifest']
    C:\src\surun\tests\TestScreenshotSuRun\TestScreenshotSuRun.vcxproj ['main.cpp', 'Header.h']
    


```python
for i in glob_files("*.vcxproj"):
    k: str = i.read_text(encoding="utf8")
    import re

    result = re.findall(r"Include=\"(.*)\"", k)
    result = list(filter(lambda x: "|" not in x, result))
    # print(i, result)
    for r in result:
        if k.count(r) != 1:
            print(i, r)
```

    C:\src\surun\PC\SuRunExtVC9.vcxproj SuRunExt.Def
    C:\src\surun\PC\SuRunExtVC9.vcxproj SuRunExt32.Def
    

## seems there are many extra paths


```python
for i in glob_files("*.vcxproj"):
    k: str = i.read_text(encoding="utf8")
    import re
    # after xml '>' then start with '.'
    result = re.findall(r">(\.+[\\/a-zA-Z\.]+)", k)
    result = list(filter(lambda x: "|" not in x, result))
    print(i, result)
```

    C:\src\surun\install_surun\InstallSuRunVC9.vcxproj ['.\\Debug\\', '.\\Debug\\', '..\\', '.\\Release\\', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\Debug/InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc', '.\\Release/InstallSuRun.tlb', '.\\Release/InstallSuRun.pch', '.\\Release/', '.\\Release/', '.\\Release/', '../InstallSuRun.exe', '.\\Release/InstallSuRun.pdb', '.\\Release/InstallSuRun.bsc', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\Debug/InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\Debug/InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc']
    C:\src\surun\PC\SuRunC.vcxproj ['.\\Debug\\', '.\\Debug\\', '.\\Release\\', '.\\Release\\', '.\\Debug/SuRunC.tlb', '.\\Debug/SuRunC.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '..\\DebugU/SuRun.com', '.\\Debug/SuRunC.pdb', '.\\Debug/SuRunC.bsc', '.\\Debug/SuRunC.tlb', '.\\Debug/SuRunC.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '..\\DebugUx', '.\\Debug/SuRunC.pdb', '.\\Debug/SuRunC.bsc', '.\\Release/SuRunC.tlb', '.\\Release/SuRunC.pch', '.\\Release/', '.\\Release/', '.\\Release/', '..\\ReleaseU/SuRun.com', '.\\Release/SuRunC.pdb', '.\\Release/SuRunC.bsc', '.\\Release/SuRunC.tlb', '.\\Release/SuRunC.pch', '.\\Release/', '.\\Release/', '.\\Release/', '..\\ReleaseUx', '.\\Release/SuRunC.pdb', '.\\Release/SuRunC.bsc', '.\\Debug/SuRunC.tlb', '.\\Debug/SuRunC.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\DebugU/SuRun.com', '.\\Debug/SuRunC.pdb', '.\\Debug/SuRunC.bsc', '.\\Debug/SuRunC.tlb', '.\\Debug/SuRunC.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '..\\DebugUx', '.\\Debug/SuRunC.pdb', '.\\Debug/SuRunC.bsc', '.\\Release/SuRunC.tlb', '.\\Release/SuRunC.pch', '.\\Release/', '.\\Release/', '.\\Release/', '.\\ReleaseU/SuRun.com', '.\\Release/SuRunC.pdb', '.\\Release/SuRunC.bsc', '.\\Release/SuRunC.tlb', '.\\Release/SuRunC.pch', '.\\Release/', '.\\Release/', '.\\Release/', '..\\ReleaseUx', '.\\Release/SuRunC.pdb', '.\\Release/SuRunC.bsc']
    C:\src\surun\PC\SuRunExtVC9.vcxproj ['.\\DebugU\\', '.\\DebugU\\', '.\\ReleaseU\\', '.\\ReleaseU\\', './ReleaseUx', './ReleaseUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\DebugUx', '.\\DebugUx', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRunExt.tlb', '.\\DebugU/SuRunExt.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '../DebugU/SuRunExt.dll', '.\\SuRunExt.Def', '.\\DebugU/SuRunExt.pdb', '.\\DebugU/SuRunExt.lib', '.\\DebugU/SuRunExt.bsc', '.\\ReleaseU/SuRunExt.tlb', '.\\ReleaseU/SuRunExt.pch', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/', '../ReleaseU/SuRunExt.dll', '.\\SuRunExt.Def', '.\\ReleaseU/SuRunExt.pdb', '.\\ReleaseU/SuRunExt.lib', '.\\ReleaseU/SuRunExt.bsc', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '../ReleaseUx', '.\\SuRunExt.Def', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '../ReleaseUx', '.\\SuRunExt', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\DebugU/SuRunExt.tlb', '.\\DebugU/SuRunExt.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '../DebugU/SuRunExt.dll', '.\\SuRunExt.Def', '.\\DebugU/SuRunExt.pdb', '.\\DebugU/SuRunExt.lib', '.\\DebugU/SuRunExt.bsc', '../DebugUx', '.\\SuRunExt.Def', '.\\DebugUx', '.\\DebugUx', './DebugUx', '.\\DebugU/SuRunExt.tlb', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '../DebugUx', '.\\SuRunExt', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRunExt.bsc']
    C:\src\surun\PC\SuRunVC9.vcxproj ['.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseU\\', '.\\ReleaseU\\', '.\\DebugU\\', '.\\DebugU\\', '.\\ReleaseUx', '.\\ReleaseUx', '.\\DebugUx', '.\\DebugUx', '.\\DebugUx', '.\\DebugUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseU/SuRun.tlb', '.\\ReleaseU/SuRun.pch', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/SuRun.exe', '.\\ReleaseU/SuRun.pdb', '.\\ReleaseU/SuRun.bsc', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/SuRun.exe', '.\\DebugU/SuRun.pdb', '.\\DebugU/SuRun.bsc', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/SuRun.exe', '.\\DebugU/SuRun.pdb', '.\\DebugU/SuRun.bsc', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRun.bsc']
    C:\src\surun\tests\TestScreenshot\TestScreenshot.vcxproj []
    C:\src\surun\tests\TestScreenshotSuRun\TestScreenshotSuRun.vcxproj []
    

what to do...


