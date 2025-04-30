cmake_minimum_required(VERSION 3.20)

project(clinkgo)
set(TARGET test)
add_executable(${TARGET} main.c)

target_link_directories(${TARGET} PUBLIC "../godylib")
target_include_directories(${TARGET} PUBLIC "../godylib")
target_link_libraries(${TARGET} lib)
