cmake_minimum_required (VERSION 2.8)
project(ocl)

set(SOURCES ${SOURCES}
    coclinit.cpp
    ocl_buffer.cpp
    ocl_device.cpp
    ocl_program.cpp
    ocl_utils.cpp
    utils/stream.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs -fPIC")

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/AMDAPPSDK-2.9-1/include")

#add_definitions(-D_DEBUG)

include_directories("./utils" "./" "/opt/AMDAPPSDK-2.9-1/include")

add_library(ocl ${SOURCES})
