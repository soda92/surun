cmake_minimum_required(VERSION 3.22)
project(cplapplet)

set(SOURCES main.c app.def app.rc)
set(TARGET myapplet)

add_library(${TARGET} SHARED ${SOURCES})

SET_TARGET_PROPERTIES(${TARGET} PROPERTIES PREFIX "")
set_target_properties(${TARGET} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)
