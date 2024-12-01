# Microsoft Developer Studio Generated NMAKE File, Based on InstallSuRun.dsp
!IF "$(CFG)" == ""
CFG=InstallSuRun - Win32 Debug
!MESSAGE No configuration specified. Defaulting to InstallSuRun - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "InstallSuRun - Win32 Release" && "$(CFG)" != "InstallSuRun - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "InstallSuRun.mak" CFG="InstallSuRun - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "InstallSuRun - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "InstallSuRun - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "InstallSuRun - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\InstallSuRun.exe"


CLEAN :
	-@erase "$(INTDIR)\InstallSuRun.obj"
	-@erase "$(INTDIR)\InstallSuRun.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "..\InstallSuRun.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\InstallSuRun.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\InstallSuRun.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\InstallSuRun.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\InstallSuRun.pdb" /machine:I386 /out:"../InstallSuRun.exe" 
LINK32_OBJS= \
	"$(INTDIR)\InstallSuRun.obj" \
	"$(INTDIR)\InstallSuRun.res"

"..\InstallSuRun.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
TargetPath=\surun\SuRun\InstallSuRun.exe
SOURCE="$(InputPath)"
PostBuild_Desc=UPXing
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\InstallSuRun.exe"
   f:\surun\SuRun\bin\setdllcharacteristics.exe +d +n \surun\SuRun\InstallSuRun.exe
	upx --lzma \surun\SuRun\InstallSuRun.exe
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "InstallSuRun - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\InstallSuRun.exe"


CLEAN :
	-@erase "$(INTDIR)\InstallSuRun.obj"
	-@erase "$(INTDIR)\InstallSuRun.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\InstallSuRun.exe"
	-@erase "$(OUTDIR)\InstallSuRun.ilk"
	-@erase "$(OUTDIR)\InstallSuRun.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\InstallSuRun.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\InstallSuRun.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\InstallSuRun.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\InstallSuRun.pdb" /debug /machine:I386 /out:"$(OUTDIR)\InstallSuRun.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\InstallSuRun.obj" \
	"$(INTDIR)\InstallSuRun.res"

"$(OUTDIR)\InstallSuRun.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("InstallSuRun.dep")
!INCLUDE "InstallSuRun.dep"
!ELSE 
!MESSAGE Warning: cannot find "InstallSuRun.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "InstallSuRun - Win32 Release" || "$(CFG)" == "InstallSuRun - Win32 Debug"
SOURCE=.\InstallSuRun.cpp

"$(INTDIR)\InstallSuRun.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\InstallSuRun.rc

"$(INTDIR)\InstallSuRun.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

