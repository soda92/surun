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
)

include_directories(. ..)

add_library(SuRunExt ${SOURCES})