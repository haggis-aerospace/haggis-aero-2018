cmake_minimum_required(VERSION 3.8)
project(cvlib)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES library.cpp library.h)
add_library(cvlib SHARED ${SOURCE_FILES})