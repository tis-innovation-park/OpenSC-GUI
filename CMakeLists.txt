cmake_minimum_required (VERSION 2.6)
project (buergerkarte)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -O0 -Wall -Wno-long-long -pedantic -Wno-vla")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory (src)

