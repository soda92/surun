---
date: 2025-05-01T01:08:21+08:00
title: '`.vcxproj` in Visual Studio'
---

https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure?view=msvc-170

If you intend to maintain your project properties in the IDE, we recommend you only create and modify your .vcxproj projects in the IDE, and avoid manual edits to the files. In most cases, you never need to manually edit the project file. Manual edits may break the project connections required to modify project settings in the Visual Studio property pages, and can cause build errors that are difficult to debug and repair. For more information about using the property pages, see Set C++ compiler and build properties in Visual Studio.


```python
from surun_tools.glob1 import glob_files
for i in glob_files("*.vcxproj"):
    print(i)
```

    C:\src\surun\src\PC\InstallSuRunVC9.vcxproj
    C:\src\surun\src\PC\SuRunExtVC9.vcxproj
    C:\src\surun\src\PC\SuRunVC9.vcxproj
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

    C:\src\surun\src\PC\InstallSuRunVC9.vcxproj ['..\\install_surun\\InstallSuRun.cpp', '..\\install_surun\\InstallSuRun.rc', '..\\install_surun\\resource.h', '..\\install_surun\\app.manifest', '..\\install_surun\\InstSuRunVer.rc2', '..\\resources\\SuRun.ico', 'SuRunExtVC9.vcxproj', 'SuRunVC9.vcxproj']
    C:\src\surun\src\PC\SuRunExtVC9.vcxproj ['..\\surun\\DBGTrace.cpp', '..\\surun\\DynWTSAPI.cpp', '..\\surun\\Helpers.cpp', '..\\surun\\IsAdmin.cpp', '..\\surun\\LogonDlg.cpp', '..\\surun\\LSALogon.cpp', '..\\surun\\lsa_laar.cpp', '..\\surun\\Setup.cpp', '..\\surun\\sspi_auth.cpp', '..\\surun\\UserGroups.cpp', '..\\surun\\WinStaDesk.cpp', '..\\surun_ext\\IATHook.cpp', '..\\surun_ext\\SuRunext.cpp', '..\\surun_ext\\SysMenuHook.cpp', 'SuRunExt.Def', 'SuRunExt32.Def', '..\\surun_ext\\SuRunExtVer.rc2', '..\\surun_ext\\SuRunext.rc', '..\\surun\\DynWTSAPI.h', '..\\surun\\Helpers.h', '..\\surun_ext\\IATHook.h', '..\\surun_ext\\resource.h', '..\\surun_ext\\SuRunext.h', '..\\surun_ext\\SysMenuHook.h', '..\\surun\\app.manifest', '..\\resources\\icon1.ico', '..\\resources\\Shield.ico', '..\\resources\\SuRun.ico']
    C:\src\surun\src\PC\SuRunVC9.vcxproj ['..\\surun\\DBGTrace.cpp', '..\\surun\\DynWTSAPI.cpp', '..\\surun\\Helpers.cpp', '..\\surun\\IsAdmin.cpp', '..\\surun\\LogonDlg.cpp', '..\\surun\\LSALogon.cpp', '..\\surun\\lsa_laar.cpp', '..\\surun\\main.cpp', '..\\surun\\ReqAdmin.cpp', '..\\surun\\Service.cpp', '..\\surun\\Setup.cpp', '..\\surun\\sspi_auth.cpp', '..\\surun\\TrayMsgWnd.cpp', '..\\surun\\TrayShowAdmin.cpp', '..\\surun\\UserGroups.cpp', '..\\surun\\WatchDog.cpp', '..\\surun\\WinStaDesk.cpp', '..\\surun\\launcher.cpp', '..\\surun\\SuRun.rc', '..\\surun\\anchor.h', '..\\surun\\main.h', '..\\surun\\CmdLine.h', '..\\surun\\DBGTrace.H', '..\\surun\\DynWTSAPI.h', '..\\surun\\Helpers.h', '..\\surun\\IsAdmin.h', '..\\surun\\LogonDlg.h', '..\\surun\\LSALogon.h', '..\\surun\\lsa_laar.h', '..\\surun\\pugxml.h', '..\\surun\\ReqAdmin.h', '..\\surun\\resource.h', '..\\surun\\ResStr.h', '..\\surun\\ScreenSnap.h', '..\\surun\\Service.h', '..\\surun\\Setup.h', '..\\surun\\sspi_auth.h', '..\\surun\\SuRunVer.h', '..\\surun\\TrayMsgWnd.h', '..\\surun\\TrayShowAdmin.h', '..\\surun\\UserGroups.h', '..\\surun\\WatchDog.h', '..\\surun\\WinStaDesk.h', '..\\resources\\Admin.ico', '..\\resources\\AutoCancel.ico', '..\\resources\\CancelWindows.ico', '..\\resources\\ico10605.ico', '..\\resources\\ico10606.ico', '..\\resources\\ico10607.ico', '..\\resources\\neverquestion.ico', '..\\resources\\NoAdmin.ico', '..\\resources\\NoQuestion.ico', '..\\resources\\NoRestrict.ico', '..\\resources\\NoWindow.ico', '..\\resources\\NoWindows.ico', '..\\resources\\Question.ico', '..\\resources\\Restrict.ico', '..\\resources\\SHADMIN.ico', '..\\resources\\Shield.ico', '..\\resources\\SrAdmin.ico', '..\\resources\\SuRun.ico', '..\\resources\\SuRunBW.ico', '..\\resources\\Windows.ico', '..\\surun\\app.manifest', '..\\surun\\SuRunVer.rc2', '..\\docs\\ChangeLog.md', '..\\docs\\gpedit.md', '..\\ReadMe.md', 'SuRunExtVC9.vcxproj']
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

    C:\src\surun\src\PC\SuRunExtVC9.vcxproj SuRunExt.Def
    C:\src\surun\src\PC\SuRunExtVC9.vcxproj SuRunExt32.Def
    

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

    C:\src\surun\src\PC\InstallSuRunVC9.vcxproj ['.\\Debug\\', '.\\Debug\\', '..\\', '.\\Release\\', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', './InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc', '.\\Release/InstallSuRun.tlb', '.\\Release/InstallSuRun.pch', '.\\Release/', '.\\Release/', '.\\Release/', '../InstallSuRun.exe', '.\\Release/InstallSuRun.pdb', '.\\Release/InstallSuRun.bsc', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\Debug/InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc', '.\\Debug/InstallSuRun.tlb', '.\\Debug/InstallSuRun.pch', '.\\Debug/', '.\\Debug/', '.\\Debug/', '.\\Debug/InstallSuRun.exe', '.\\Debug/InstallSuRun.pdb', '.\\Debug/InstallSuRun.bsc']
    C:\src\surun\src\PC\SuRunExtVC9.vcxproj ['.\\DebugU\\', '.\\DebugU\\', '.\\ReleaseU\\', '.\\ReleaseU\\', './ReleaseUx', './ReleaseUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\DebugUx', '.\\DebugUx', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRunExt.tlb', '.\\DebugU/SuRunExt.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '..\\surun', '.\\DebugU/SuRunExt.pdb', '.\\DebugU/SuRunExt.lib', '.\\DebugU/SuRunExt.bsc', '.\\ReleaseU/SuRunExt.tlb', '.\\ReleaseU/SuRunExt.pch', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/', '../ReleaseU/SuRunExt.dll', '..\\surun', '.\\ReleaseU/SuRunExt.pdb', '.\\ReleaseU/SuRunExt.lib', '.\\ReleaseU/SuRunExt.bsc', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '../ReleaseUx', '..\\surun', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '../ReleaseUx', '../surun', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\DebugU/SuRunExt.tlb', '.\\DebugU/SuRunExt.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '..\\surun', '.\\DebugU/SuRunExt.pdb', '.\\DebugU/SuRunExt.lib', '.\\DebugU/SuRunExt.bsc', '..\\surun', '.\\DebugUx', '.\\DebugUx', './DebugUx', '.\\DebugU/SuRunExt.tlb', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '../surun', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRunExt.bsc']
    C:\src\surun\src\PC\SuRunVC9.vcxproj ['.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseU\\', '.\\ReleaseU\\', '.\\DebugU\\', '.\\DebugU\\', '.\\ReleaseUx', '.\\ReleaseUx', '.\\DebugUx', '.\\DebugUx', '.\\DebugUx', '.\\DebugUx', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseUsr', '.\\ReleaseU/SuRun.tlb', '.\\ReleaseU/SuRun.pch', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/', '.\\ReleaseU/SuRun.exe', '.\\ReleaseU/SuRun.pdb', '.\\ReleaseU/SuRun.bsc', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/SuRun.exe', '.\\DebugU/SuRun.pdb', '.\\DebugU/SuRun.bsc', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\ReleaseUx', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/', '.\\DebugU/SuRun.exe', '.\\DebugU/SuRun.pdb', '.\\DebugU/SuRun.bsc', '.\\DebugU/SuRun.tlb', '.\\DebugU/SuRun.pch', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugUsr', '.\\DebugU/SuRun.bsc']
    C:\src\surun\tests\TestScreenshot\TestScreenshot.vcxproj []
    C:\src\surun\tests\TestScreenshotSuRun\TestScreenshotSuRun.vcxproj []
    

what to do...
