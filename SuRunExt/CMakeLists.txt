add_definitions(-D_UNICODE -DUNICODE -DWIN32 -D_WINDOWS)
add_compile_options(-Wno-write-strings -Wno-register -Wno-narrowing
-Wno-pointer-arith -Wno-conversion-null)

set(SOURCES
"../DBGTrace.cpp"
"../DynWTSAPI.cpp"
"../Helpers.cpp"
"../IsAdmin.cpp"
"../LogonDlg.cpp"
"../LSALogon.cpp"
"../lsa_laar.cpp"
"../Setup.cpp"
"../sspi_auth.cpp"
"../UserGroups.cpp"
"../WinStaDesk.cpp"
"IATHook.cpp"
"SuRunext.cpp"
"SysMenuHook.cpp"

SuRunExt.rc
)

include_directories(. ..)

add_library(SuRunExt SHARED ${SOURCES})
SET_TARGET_PROPERTIES(SuRunExt PROPERTIES PREFIX "")

target_link_libraries(SuRunExt PRIVATE winmm.lib Psapi.lib Netapi32.lib 
Shlwapi.lib Secur32.lib Mpr.lib Userenv.lib advapi32.lib 
ole32.lib shell32.lib version.lib comctl32.lib kernel32.lib
rpcrt4.lib msi.lib crypt32.lib
)