cmake_minimum_required(VERSION 3.8)
project(cvlib)

set(FastText_DIR ${FASTTEXT_DIR})
message(${FASTTEXT_DIR}})
set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
find_package(FastText REQUIRED)

add_subdirectory (lib)
add_subdirectory (test)

