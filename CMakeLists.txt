cmake_minimum_required (VERSION 2.8.11)
project (buergerkarte)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g2 -O0 -Wall -Wextra -pedantic")
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory (src)

