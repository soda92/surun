cmake_minimum_required(VERSION 3.23)
project("aaa")

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wno-write-strings -Wno-register)

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
"WinStaDesk.cpp")

add_definitions(-D_UNICODE -DUNICODE -DWIN32 -D_WINDOWS)
# include_directories(C:/TDM-GCC-64/x86_64-w64-mingw32/include C:/TDM-GCC-64/lib/gcc/x86_64-w64-mingw32/10.3.0/include/c++/x86_64-w64-mingw32)
add_executable(aaa ${SOURCES})
target_link_libraries(aaa PRIVATE winmm.lib Psapi.lib Netapi32.lib Shlwapi.lib)