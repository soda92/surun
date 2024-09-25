# Microsoft Developer Studio Generated NMAKE File, Based on SuRun.dsp
!IF "$(CFG)" == ""
CFG=SuRun - Win32 Unicode Release
!MESSAGE No configuration specified. Defaulting to SuRun - Win32 Unicode Release.
!ENDIF 

!IF "$(CFG)" != "SuRun - Win32 Unicode Debug" && "$(CFG)" != "SuRun - Win32 Unicode Release" && "$(CFG)" != "SuRun - Win32 x64 Unicode Release" && "$(CFG)" != "SuRun - Win32 SuRun32 Unicode Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SuRun.mak" CFG="SuRun - Win32 Unicode Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SuRun - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "SuRun - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "SuRun - Win32 x64 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "SuRun - Win32 SuRun32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SuRun - Win32 Unicode Debug"

OUTDIR=.\DebugU
INTDIR=.\DebugU
# Begin Custom Macros
OutDir=.\DebugU
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SuRun.exe"

!ELSE 

ALL : "SuRunExt - Win32 Unicode Debug" "$(OUTDIR)\SuRun.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SuRunExt - Win32 Unicode DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\ReqAdmin.obj"
	-@erase "$(INTDIR)\Service.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRun.res"
	-@erase "$(INTDIR)\TrayMsgWnd.obj"
	-@erase "$(INTDIR)\TrayShowAdmin.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WatchDog.obj"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRun.exe"
	-@erase "$(OUTDIR)\SuRun.ilk"
	-@erase "$(OUTDIR)\SuRun.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /Fp"$(INTDIR)\SuRun.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRun.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRun.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib user32.lib advapi32.lib kernel32.lib shell32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SuRun.pdb" /debug /machine:I386 /out:"$(OUTDIR)\SuRun.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ReqAdmin.obj" \
	"$(INTDIR)\Service.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\TrayMsgWnd.obj" \
	"$(INTDIR)\TrayShowAdmin.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WatchDog.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRun.res" \
	".\SuRunExt\DebugU\SuRunExt.lib"

"$(OUTDIR)\SuRun.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   if exist .\DebugU\SuRun.exe del /f .\DebugU\SuRun.exe 1>NUL 2>NUL
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\DebugU
SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "SuRun - Win32 Unicode Release"

OUTDIR=.\ReleaseU
INTDIR=.\ReleaseU
# Begin Custom Macros
OutDir=.\ReleaseU
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SuRun.exe"

!ELSE 

ALL : "SuRunExt - Win32 Unicode Release" "$(OUTDIR)\SuRun.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SuRunExt - Win32 Unicode ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\ReqAdmin.obj"
	-@erase "$(INTDIR)\Service.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRun.res"
	-@erase "$(INTDIR)\TrayMsgWnd.obj"
	-@erase "$(INTDIR)\TrayShowAdmin.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WatchDog.obj"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRun.exe"
	-@erase "$(OUTDIR)\SuRun.map"
	-@erase "$(OUTDIR)\SuRun.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\SuRun.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRun.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRun.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib user32.lib advapi32.lib kernel32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SuRun.pdb" /map:"$(INTDIR)\SuRun.map" /debug /machine:I386 /out:"$(OUTDIR)\SuRun.exe" /IGNORE:4089 /OPT:REF 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ReqAdmin.obj" \
	"$(INTDIR)\Service.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\TrayMsgWnd.obj" \
	"$(INTDIR)\TrayShowAdmin.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WatchDog.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRun.res" \
	".\SuRunExt\ReleaseU\SuRunExt.lib"

"$(OUTDIR)\SuRun.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   if exist .\ReleaseU\SuRun.exe del /f .\ReleaseU\SuRun.exe 1>NUL 2>NUL
	if exist .\ReleaseU\SuRun.exe .\ReleaseU\SuRun.exe /DeleteService 1>NUL 2>NUL
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseU
WkspDir=.
TargetPath=.\ReleaseU\SuRun.exe
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\ReleaseU
# End Custom Macros

$(DS_POSTBUILD_DEP) : "SuRunExt - Win32 Unicode Release" "$(OUTDIR)\SuRun.exe"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n .\ReleaseU\SuRun.exe
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "SuRun - Win32 x64 Unicode Release"

OUTDIR=.\ReleaseUx64
INTDIR=.\ReleaseUx64
# Begin Custom Macros
OutDir=.\ReleaseUx64
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SuRun.exe"

!ELSE 

ALL : "SuRunExt - Win32 x64 Unicode Release" "$(OUTDIR)\SuRun.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SuRunExt - Win32 x64 Unicode ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\ReqAdmin.obj"
	-@erase "$(INTDIR)\Service.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRun.res"
	-@erase "$(INTDIR)\TrayMsgWnd.obj"
	-@erase "$(INTDIR)\TrayShowAdmin.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WatchDog.obj"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRun.exe"
	-@erase "$(OUTDIR)\SuRun.map"
	-@erase "$(OUTDIR)\SuRun.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /EHsc /Wp64 /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRun.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRun.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib user32.lib advapi32.lib kernel32.lib shell32.lib bufferoverflowu.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SuRun.pdb" /map:"$(INTDIR)\SuRun.map" /debug /machine:IX86 /out:"$(OUTDIR)\SuRun.exe" /IGNORE:4089 /machine:AMD64 /OPT:REF 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ReqAdmin.obj" \
	"$(INTDIR)\Service.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\TrayMsgWnd.obj" \
	"$(INTDIR)\TrayShowAdmin.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WatchDog.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRun.res" \
	".\SuRunExt\ReleaseUx64\SuRunExt.lib"

"$(OUTDIR)\SuRun.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   if exist .\ReleaseUx64\SuRun.exe del /f .\ReleaseUx64\SuRun.exe 1>NUL 2>NUL
	if exist .\ReleaseUx64\SuRun.exe .\ReleaseUx64\SuRun.exe /DeleteService 1>NUL 2>NUL
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseUx64
WkspDir=.
TargetPath=.\ReleaseUx64\SuRun.exe
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\ReleaseUx64
# End Custom Macros

$(DS_POSTBUILD_DEP) : "SuRunExt - Win32 x64 Unicode Release" "$(OUTDIR)\SuRun.exe"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n .\ReleaseUx64\SuRun.exe
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "SuRun - Win32 SuRun32 Unicode Release"

OUTDIR=.\ReleaseUsr32
INTDIR=.\ReleaseUsr32

!IF "$(RECURSE)" == "0" 

ALL : ".\ReleaseUx64\SuRun32.bin"

!ELSE 

ALL : "SuRunExt - Win32 SuRun32 Unicode Release" ".\ReleaseUx64\SuRun32.bin"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"SuRunExt - Win32 SuRun32 Unicode ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\ReqAdmin.obj"
	-@erase "$(INTDIR)\Service.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRun32.res"
	-@erase "$(INTDIR)\TrayMsgWnd.obj"
	-@erase "$(INTDIR)\TrayShowAdmin.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WatchDog.obj"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRun32.map"
	-@erase "$(OUTDIR)\SuRun32.pdb"
	-@erase ".\ReleaseUx64\SuRun32.bin"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_SR32" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\SuRun.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRun32.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRun32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=gdi32.lib user32.lib advapi32.lib kernel32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SuRun32.pdb" /map:"$(INTDIR)\SuRun32.map" /debug /machine:I386 /out:"ReleaseUx64/SuRun32.bin" /IGNORE:4089 /OPT:REF 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ReqAdmin.obj" \
	"$(INTDIR)\Service.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\TrayMsgWnd.obj" \
	"$(INTDIR)\TrayShowAdmin.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WatchDog.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRun32.res" \
	".\SuRunExt\ReleaseUsr32\SuRunExt32.lib"

".\ReleaseUx64\SuRun32.bin" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
TargetPath=.\ReleaseUx64\SuRun32.bin
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "SuRunExt - Win32 SuRun32 Unicode Release" ".\ReleaseUx64\SuRun32.bin"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n .\ReleaseUx64\SuRun32.bin
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SuRun.dep")
!INCLUDE "SuRun.dep"
!ELSE 
!MESSAGE Warning: cannot find "SuRun.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SuRun - Win32 Unicode Debug" || "$(CFG)" == "SuRun - Win32 Unicode Release" || "$(CFG)" == "SuRun - Win32 x64 Unicode Release" || "$(CFG)" == "SuRun - Win32 SuRun32 Unicode Release"
SOURCE=.\DBGTrace.cpp

"$(INTDIR)\DBGTrace.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DynWTSAPI.cpp

"$(INTDIR)\DynWTSAPI.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Helpers.cpp

"$(INTDIR)\Helpers.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\IsAdmin.cpp

"$(INTDIR)\IsAdmin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LogonDlg.cpp

"$(INTDIR)\LogonDlg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lsa_laar.cpp

"$(INTDIR)\lsa_laar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LSALogon.cpp

"$(INTDIR)\LSALogon.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ReqAdmin.cpp

"$(INTDIR)\ReqAdmin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Service.cpp

"$(INTDIR)\Service.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Setup.cpp

"$(INTDIR)\Setup.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sspi_auth.cpp

"$(INTDIR)\sspi_auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SuRun.rc

!IF  "$(CFG)" == "SuRun - Win32 Unicode Debug"


"$(INTDIR)\SuRun.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRun - Win32 Unicode Release"


"$(INTDIR)\SuRun.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRun - Win32 x64 Unicode Release"


"$(INTDIR)\SuRun.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRun - Win32 SuRun32 Unicode Release"


"$(INTDIR)\SuRun32.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\TrayMsgWnd.cpp

"$(INTDIR)\TrayMsgWnd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TrayShowAdmin.cpp

"$(INTDIR)\TrayShowAdmin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\UserGroups.cpp

"$(INTDIR)\UserGroups.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WatchDog.cpp

"$(INTDIR)\WatchDog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WinStaDesk.cpp

"$(INTDIR)\WinStaDesk.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "SuRun - Win32 Unicode Debug"

"SuRunExt - Win32 Unicode Debug" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 Unicode Debug" 
   cd ".."

"SuRunExt - Win32 Unicode DebugCLEAN" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 Unicode Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "SuRun - Win32 Unicode Release"

"SuRunExt - Win32 Unicode Release" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 Unicode Release" 
   cd ".."

"SuRunExt - Win32 Unicode ReleaseCLEAN" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 Unicode Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "SuRun - Win32 x64 Unicode Release"

"SuRunExt - Win32 x64 Unicode Release" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 x64 Unicode Release" 
   cd ".."

"SuRunExt - Win32 x64 Unicode ReleaseCLEAN" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 x64 Unicode Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "SuRun - Win32 SuRun32 Unicode Release"

"SuRunExt - Win32 SuRun32 Unicode Release" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 SuRun32 Unicode Release" 
   cd ".."

"SuRunExt - Win32 SuRun32 Unicode ReleaseCLEAN" : 
   cd ".\SuRunExt"
   $(MAKE) /$(MAKEFLAGS) /F .\SuRunExt.mak CFG="SuRunExt - Win32 SuRun32 Unicode Release" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 


!ENDIF 

