cmake_minimum_required(VERSION 3.0.0)

project(motion_monitor)
set(CMAKE_CXX_STANDARD 11)



file(GLOB SRC

 MyCode/Src/*.c
 MyCode/Src/*.cpp
 )

include_directories ( MyCode/Inc)
include_directories ( Inc)
add_executable(motion_monitor 
 ${SRC}
 )


