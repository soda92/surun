cmake_minimum_required(VERSION 3.23)
project("aaa")

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wno-write-strings -Wno-register
-Wno-pointer-arith -Wno-conversion-null)
add_subdirectory(SuRunExt)

set(SOURCES "DBGTrace.cpp"
"DynWTSAPI.cpp"
"Helpers.cpp"
"IsAdmin.cpp"
"LogonDlg.cpp"
"LSALogon.cpp"
"lsa_laar.cpp"
"main.cpp"
"ReqAdmin.cpp"
"Service.cpp"
"Setup.cpp"
"sspi_auth.cpp"
"TrayMsgWnd.cpp"
"TrayShowAdmin.cpp"
"UserGroups.cpp"
"WatchDog.cpp"
"WinStaDesk.cpp"
SuRun.rc
)

add_definitions(-D_UNICODE -DUNICODE -DWIN32 -D_WINDOWS)

if(DEFINED SKIP_WINDIR)
add_definitions(-DSKIP_WINDIR -D_DEBUG)
endif()

add_executable(SuRun ${SOURCES})
target_link_libraries(SuRun PRIVATE winmm.lib Psapi.lib Netapi32.lib 
Shlwapi.lib Secur32.lib Mpr.lib Userenv.lib advapi32.lib 
ole32.lib shell32.lib version.lib comctl32.lib kernel32.lib
rpcrt4.lib msi.lib crypt32.lib
SuRunExt
)
# SuRunExt/ReleaseUx64/SuRunExt.lib)