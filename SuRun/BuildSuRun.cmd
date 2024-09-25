@echo off
rem goto VC6compile
if NOT "%MSDevDir%"=="" goto VC6compile
if NOT "%VS80COMNTOOLS%"=="" goto VC8compile
if NOT "%VS90COMNTOOLS%"=="" goto VC9compile

rem compile using VC6
:VC6compile
echo VC6
if "%MSSDK%"=="" SET MSSDK=E:\MSTOOLS
SET VC6Dir=%MSDevDir%\..\..
if "%VC6Dir%"=="\..\.." SET VC6Dir=E:\VStudio

SETLOCAL
call %VC6Dir%\VC98\Bin\VCVARS32.BAT
%VC6Dir%\Common\MSDev98\Bin\MSDEV.com /useenv SuRun.dsw /MAKE ALL /CLEAN
call %MSSDK%\SetEnv.Cmd /X64 /RETAIL
echo building SuRunX64
%VC6Dir%\Common\MSDev98\Bin\MSDEV.com /useenv SuRun.dsw /MAKE "SuRun - Win32 x64 Unicode Release"
ENDLOCAL

SETLOCAL
set MSVCDir=%VC6Dir%\VC98
set DevEnvDir=%VC6Dir%\Common\IDE
call %VC6Dir%\VC98\Bin\VCVARS32.BAT
call %MSSDK%\SetEnv.Cmd /2000 /RETAIL
set MSVCVer=6.0
echo building SuRun32
%VC6Dir%\Common\MSDev98\Bin\MSDEV.com /useenv SuRun.dsw /MAKE "SuRun - Win32 Unicode Release" "SuRun - Win32 SuRun32 Unicode Release" "InstallSuRun - Win32 Release"
ENDLOCAL
goto Done

rem compile using VC8 (2005), VC9 (2008) or VC10 (2010)
:VC8compile
SETLOCAL
echo VC8
call "%VS80COMNTOOLS%vsvars32.bat"
msbuild SuRun.sln /t:clean 1>NUL 2>NUL
msbuild SuRun.sln /t:Rebuild /p:Configuration="x64 Unicode Release" /p:Platform=x64
msbuild SuRun.sln /t:Rebuild /p:Configuration="SuRun32 Unicode Release" /p:Platform=Win32
msbuild SuRun.sln /t:Rebuild /p:Configuration="Unicode Release" /p:Platform=Win32
ENDLOCAL
goto Done

:VC9compile
echo VC9
SETLOCAL
call "%VS90COMNTOOLS%vsvars32.bat"
msbuild SuRunVC9.sln /t:clean 1>NUL 2>NUL
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="x64 Unicode Release" /p:Platform=x64
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="SuRun32 Unicode Release" /p:Platform=Win32
msbuild SuRunVC9.sln /t:Rebuild /p:Configuration="Unicode Release" /p:Platform=Win32

SuRun InstallSuRun.exe /RenameYourselfToYourVersion

ENDLOCAL

:Done
pause
