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

set(TARGET SuRunExt)
if(DEFINED ARCH_WIN32)
    set(TARGET SuRunExt32)
endif()

add_library(${TARGET} SHARED ${SOURCES})
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES PREFIX "")

target_link_libraries(${TARGET} PRIVATE winmm.lib Psapi.lib Netapi32.lib
Shlwapi.lib Secur32.lib Mpr.lib Userenv.lib advapi32.lib
ole32.lib shell32.lib version.lib comctl32.lib kernel32.lib
rpcrt4.lib msi.lib crypt32.lib
)
