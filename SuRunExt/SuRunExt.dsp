# Microsoft Developer Studio Project File - Name="SuRunExt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SURUNEXT - WIN32 UNICODE RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SuRunExt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SuRunExt.mak" CFG="SURUNEXT - WIN32 UNICODE RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SuRunExt - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SuRunExt - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SuRunExt - Win32 x64 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SuRunExt - Win32 SuRun32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SuRunExt - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseU"
# PROP BASE Intermediate_Dir "ReleaseU"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseU"
# PROP Intermediate_Dir "ReleaseU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /FAcs /YX /FD /c
# SUBTRACT CPP /WX /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /map /debug /machine:I386 /def:".\SuRunExt.Def" /out:"../ReleaseU/SuRunExt.dll" /OPT:REF /IGNORE:4089
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetPath=..\ReleaseU\SuRunExt.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Applying Certificate
PostBuild_Cmds=if exist $(SolutionDir)..\Cert\certify.cmd $(SolutionDir)..\Cert\certify.cmd $(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugU"
# PROP BASE Intermediate_Dir "DebugU"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"../DebugU/SuRunExt.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetPath=\SuRun\SuRun\DebugU\SuRunExt.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Applying Certificate
PostBuild_Cmds=if exist $(SolutionDir)..\Cert\certify.cmd $(SolutionDir)..\Cert\certify.cmd $(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 x64 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseUx64"
# PROP BASE Intermediate_Dir "ReleaseUx64"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUx64"
# PROP Intermediate_Dir "ReleaseUx64"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /FAcs /FD /EHsc /Wp64 /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:I386 /out:"../ReleaseU/SuRunExt.dll" /IGNORE:4089
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 bufferoverflowu.lib /nologo /dll /map /debug /machine:IX86 /def:".\SuRunExt.Def" /out:"../ReleaseUx64/SuRunExt.dll" /OPT:REF /IGNORE:4089 /machine:AMD64
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetPath=..\ReleaseUx64\SuRunExt.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Applying Certificate
PostBuild_Cmds=if exist $(SolutionDir)..\Cert\certify.cmd $(SolutionDir)..\Cert\certify.cmd $(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 SuRun32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseUsr32"
# PROP BASE Intermediate_Dir "ReleaseUsr32"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseUsr32"
# PROP Intermediate_Dir "ReleaseUsr32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /D "_SR32" /FAcs /YX /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /fo"ReleaseUsr32/SuRunext32.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"ReleaseUsr32/SuRunExt32.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:I386 /out:"ReleaseUsr32/SuRunExt32.dll" /IGNORE:4089
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /dll /map /debug /machine:I386 /def:".\SuRunExt32.Def" /out:"../ReleaseUx64/SuRunExt32.dll" /IGNORE:4089 /OPT:REF
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetPath=..\ReleaseUx64\SuRunExt32.dll
SOURCE="$(InputPath)"
PostBuild_Desc=Applying Certificate
PostBuild_Cmds=if exist $(SolutionDir)..\Cert\certify.cmd $(SolutionDir)..\Cert\certify.cmd $(TargetPath)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SuRunExt - Win32 Unicode Release"
# Name "SuRunExt - Win32 Unicode Debug"
# Name "SuRunExt - Win32 x64 Unicode Release"
# Name "SuRunExt - Win32 SuRun32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\DBGTrace.cpp
# End Source File
# Begin Source File

SOURCE=..\DynWTSAPI.cpp
# End Source File
# Begin Source File

SOURCE=..\Helpers.cpp
# End Source File
# Begin Source File

SOURCE=.\IATHook.cpp
# End Source File
# Begin Source File

SOURCE=..\IsAdmin.cpp
# End Source File
# Begin Source File

SOURCE=..\LogonDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\lsa_laar.cpp
# End Source File
# Begin Source File

SOURCE=..\LSALogon.cpp
# End Source File
# Begin Source File

SOURCE=..\Setup.cpp
# End Source File
# Begin Source File

SOURCE=..\sspi_auth.cpp
# End Source File
# Begin Source File

SOURCE=.\SuRunext.cpp
# End Source File
# Begin Source File

SOURCE=.\SuRunExt.Def

!IF  "$(CFG)" == "SuRunExt - Win32 Unicode Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 x64 Unicode Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 SuRun32 Unicode Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SuRunext.rc
# End Source File
# Begin Source File

SOURCE=.\SuRunExt32.Def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\SysMenuHook.cpp
# End Source File
# Begin Source File

SOURCE=..\UserGroups.cpp
# End Source File
# Begin Source File

SOURCE=..\WinStaDesk.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Helpers.h
# End Source File
# Begin Source File

SOURCE=.\IATHook.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SuRunext.h
# End Source File
# Begin Source File

SOURCE=.\SysMenuHook.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\res\SuRun.ico
# End Source File
# End Group
# Begin Source File

SOURCE=..\res\app.manifest
# End Source File
# End Target
# End Project
