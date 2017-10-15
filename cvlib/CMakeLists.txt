cmake_minimum_required(VERSION 3.8)
project(cvlib)

set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )

add_subdirectory (lib)
add_subdirectory (test)

