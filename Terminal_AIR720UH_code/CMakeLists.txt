cmake_minimum_required(VERSION 3.0.0)

project(motion_monitor)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

file(GLOB SRC

 MyCode/Src/*.c
 )

include_directories ( MyCode/Inc)
include_directories ( Inc)
add_executable(motion_monitor 
 ${SRC}
 )


