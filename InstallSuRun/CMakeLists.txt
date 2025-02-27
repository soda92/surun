cmake_minimum_required(VERSION 3.23)
project("ISR")

add_compile_options(-Wno-write-strings -Wno-register
-Wno-pointer-arith -Wno-conversion-null)

include_directories(.)

add_executable(InstallSuRun InstallSuRun.cpp InstallSuRun.rc)

target_link_libraries(InstallSuRun PRIVATE winmm.lib Psapi.lib Netapi32.lib 
Shlwapi.lib Secur32.lib Mpr.lib Userenv.lib advapi32.lib
ole32.lib shell32.lib version.lib comctl32.lib kernel32.lib
rpcrt4.lib msi.lib crypt32.lib mingwex.lib msvcrt.lib)

if(DEFINED MSYS2)
target_link_options(InstallSuRun PRIVATE "-lmingwex")
endif()

set_target_properties(InstallSuRun PROPERTIES
WIN32_EXECUTABLE "TRUE"
RESOURCE "app.manifest"
)
