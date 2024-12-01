# Microsoft Developer Studio Generated NMAKE File, Based on SuRunExt.dsp
!IF "$(CFG)" == ""
CFG=SURUNEXT - WIN32 UNICODE RELEASE
!MESSAGE No configuration specified. Defaulting to SURUNEXT - WIN32 UNICODE RELEASE.
!ENDIF 

!IF "$(CFG)" != "SuRunExt - Win32 Unicode Release" && "$(CFG)" != "SuRunExt - Win32 Unicode Debug" && "$(CFG)" != "SuRunExt - Win32 x64 Unicode Release" && "$(CFG)" != "SuRunExt - Win32 SuRun32 Unicode Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SuRunExt - Win32 Unicode Release"

OUTDIR=.\ReleaseU
INTDIR=.\ReleaseU

ALL : "..\ReleaseU\SuRunExt.dll"


CLEAN :
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IATHook.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRunext.obj"
	-@erase "$(INTDIR)\SuRunext.res"
	-@erase "$(INTDIR)\SysMenuHook.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRunExt.exp"
	-@erase "$(OUTDIR)\SuRunExt.lib"
	-@erase "$(OUTDIR)\SuRunExt.map"
	-@erase "$(OUTDIR)\SuRunExt.pdb"
	-@erase "..\ReleaseU\SuRunExt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\SuRunExt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRunext.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRunExt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /incremental:no /pdb:"$(OUTDIR)\SuRunExt.pdb" /map:"$(INTDIR)\SuRunExt.map" /debug /machine:I386 /def:".\SuRunExt.Def" /out:"../ReleaseU/SuRunExt.dll" /implib:"$(OUTDIR)\SuRunExt.lib" /OPT:REF /IGNORE:4089 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IATHook.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\SuRunext.obj" \
	"$(INTDIR)\SysMenuHook.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRunext.res"

"..\ReleaseU\SuRunExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
TargetPath=..\ReleaseU\SuRunExt.dll
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\ReleaseU\SuRunExt.dll"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n ..\ReleaseU\SuRunExt.dll
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 Unicode Debug"

OUTDIR=.\DebugU
INTDIR=.\DebugU

ALL : "..\DebugU\SuRunExt.dll"


CLEAN :
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IATHook.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRunext.obj"
	-@erase "$(INTDIR)\SuRunext.res"
	-@erase "$(INTDIR)\SysMenuHook.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRunExt.exp"
	-@erase "$(OUTDIR)\SuRunExt.lib"
	-@erase "$(OUTDIR)\SuRunExt.pdb"
	-@erase "..\DebugU\SuRunExt.dll"
	-@erase "..\DebugU\SuRunExt.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /Fp"$(INTDIR)\SuRunExt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRunext.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRunExt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /incremental:yes /pdb:"$(OUTDIR)\SuRunExt.pdb" /debug /machine:I386 /def:".\SuRunExt.Def" /out:"../DebugU/SuRunExt.dll" /implib:"$(OUTDIR)\SuRunExt.lib" /pdbtype:sept 
DEF_FILE= \
	".\SuRunExt.Def"
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IATHook.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\SuRunext.obj" \
	"$(INTDIR)\SysMenuHook.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRunext.res"

"..\DebugU\SuRunExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 x64 Unicode Release"

OUTDIR=.\ReleaseUx64
INTDIR=.\ReleaseUx64

ALL : "..\ReleaseUx64\SuRunExt.dll"


CLEAN :
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IATHook.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRunext.obj"
	-@erase "$(INTDIR)\SuRunext.res"
	-@erase "$(INTDIR)\SysMenuHook.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRunExt.exp"
	-@erase "$(OUTDIR)\SuRunExt.lib"
	-@erase "$(OUTDIR)\SuRunExt.map"
	-@erase "$(OUTDIR)\SuRunExt.pdb"
	-@erase "..\ReleaseUx64\SuRunExt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /EHsc /Wp64 /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRunext.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRunExt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=bufferoverflowu.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\SuRunExt.pdb" /map:"$(INTDIR)\SuRunExt.map" /debug /machine:IX86 /def:".\SuRunExt.Def" /out:"../ReleaseUx64/SuRunExt.dll" /implib:"$(OUTDIR)\SuRunExt.lib" /OPT:REF /IGNORE:4089 /machine:AMD64 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IATHook.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\SuRunext.obj" \
	"$(INTDIR)\SysMenuHook.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRunext.res"

"..\ReleaseUx64\SuRunExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
TargetPath=..\ReleaseUx64\SuRunExt.dll
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\ReleaseUx64\SuRunExt.dll"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n ..\ReleaseUx64\SuRunExt.dll
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "SuRunExt - Win32 SuRun32 Unicode Release"

OUTDIR=.\ReleaseUsr32
INTDIR=.\ReleaseUsr32

ALL : "..\ReleaseUx64\SuRunExt32.dll"


CLEAN :
	-@erase "$(INTDIR)\DBGTrace.obj"
	-@erase "$(INTDIR)\DynWTSAPI.obj"
	-@erase "$(INTDIR)\Helpers.obj"
	-@erase "$(INTDIR)\IATHook.obj"
	-@erase "$(INTDIR)\IsAdmin.obj"
	-@erase "$(INTDIR)\LogonDlg.obj"
	-@erase "$(INTDIR)\lsa_laar.obj"
	-@erase "$(INTDIR)\LSALogon.obj"
	-@erase "$(INTDIR)\Setup.obj"
	-@erase "$(INTDIR)\sspi_auth.obj"
	-@erase "$(INTDIR)\SuRunext.obj"
	-@erase "$(INTDIR)\SuRunext32.res"
	-@erase "$(INTDIR)\SysMenuHook.obj"
	-@erase "$(INTDIR)\UserGroups.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinStaDesk.obj"
	-@erase "$(OUTDIR)\SuRunExt32.exp"
	-@erase "$(OUTDIR)\SuRunExt32.lib"
	-@erase "$(OUTDIR)\SuRunExt32.map"
	-@erase "$(OUTDIR)\SuRunExt32.pdb"
	-@erase "..\ReleaseUx64\SuRunExt32.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D "SuRunEXT_EXPORTS" /D "_SR32" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\SuRunExt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\SuRunext32.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SuRunExt32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /incremental:no /pdb:"$(OUTDIR)\SuRunExt32.pdb" /map:"$(INTDIR)\SuRunExt32.map" /debug /machine:I386 /def:".\SuRunExt32.Def" /out:"../ReleaseUx64/SuRunExt32.dll" /implib:"$(OUTDIR)\SuRunExt32.lib" /IGNORE:4089 /OPT:REF 
LINK32_OBJS= \
	"$(INTDIR)\DBGTrace.obj" \
	"$(INTDIR)\DynWTSAPI.obj" \
	"$(INTDIR)\Helpers.obj" \
	"$(INTDIR)\IATHook.obj" \
	"$(INTDIR)\IsAdmin.obj" \
	"$(INTDIR)\LogonDlg.obj" \
	"$(INTDIR)\lsa_laar.obj" \
	"$(INTDIR)\LSALogon.obj" \
	"$(INTDIR)\Setup.obj" \
	"$(INTDIR)\sspi_auth.obj" \
	"$(INTDIR)\SuRunext.obj" \
	"$(INTDIR)\SysMenuHook.obj" \
	"$(INTDIR)\UserGroups.obj" \
	"$(INTDIR)\WinStaDesk.obj" \
	"$(INTDIR)\SuRunext32.res"

"..\ReleaseUx64\SuRunExt32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
TargetPath=..\ReleaseUx64\SuRunExt32.dll
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\ReleaseUx64\SuRunExt32.dll"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n ..\ReleaseUx64\SuRunExt32.dll
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SuRunExt.dep")
!INCLUDE "SuRunExt.dep"
!ELSE 
!MESSAGE Warning: cannot find "SuRunExt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SuRunExt - Win32 Unicode Release" || "$(CFG)" == "SuRunExt - Win32 Unicode Debug" || "$(CFG)" == "SuRunExt - Win32 x64 Unicode Release" || "$(CFG)" == "SuRunExt - Win32 SuRun32 Unicode Release"
SOURCE=..\DBGTrace.cpp

"$(INTDIR)\DBGTrace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\DynWTSAPI.cpp

"$(INTDIR)\DynWTSAPI.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Helpers.cpp

"$(INTDIR)\Helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\IATHook.cpp

"$(INTDIR)\IATHook.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\IsAdmin.cpp

"$(INTDIR)\IsAdmin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\LogonDlg.cpp

"$(INTDIR)\LogonDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\lsa_laar.cpp

"$(INTDIR)\lsa_laar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\LSALogon.cpp

"$(INTDIR)\LSALogon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Setup.cpp

"$(INTDIR)\Setup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\sspi_auth.cpp

"$(INTDIR)\sspi_auth.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\SuRunext.cpp

"$(INTDIR)\SuRunext.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SuRunext.rc

!IF  "$(CFG)" == "SuRunExt - Win32 Unicode Release"


"$(INTDIR)\SuRunext.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRunExt - Win32 Unicode Debug"


"$(INTDIR)\SuRunext.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRunExt - Win32 x64 Unicode Release"


"$(INTDIR)\SuRunext.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "SuRunExt - Win32 SuRun32 Unicode Release"


"$(INTDIR)\SuRunext32.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\SysMenuHook.cpp

"$(INTDIR)\SysMenuHook.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\UserGroups.cpp

"$(INTDIR)\UserGroups.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\WinStaDesk.cpp

"$(INTDIR)\WinStaDesk.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

