cmake_minimum_required(VERSION 3.0)
project(MinimalistBlockTrees)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O9")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")


add_subdirectory(external)
include_directories(${sdsl_SOURCE_DIR}/include)

include_directories(includes)
add_subdirectory(src)

add_subdirectory(tests)
add_subdirectory(executables)