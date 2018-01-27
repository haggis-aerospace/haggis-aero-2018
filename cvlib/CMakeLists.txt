cmake_minimum_required(VERSION 3.8)
project(cvlib)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

find_package( OpenCV REQUIRED )

add_subdirectory (lib)
add_subdirectory (test)
add_subdirectory (calibration)
add_subdirectory (network)
