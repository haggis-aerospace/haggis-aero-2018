cmake_minimum_required(VERSION 3.8)
project(configuration)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
find_package( OpenCV REQUIRED )

set(SOURCE_FILES settings.cpp fileIO.cpp fileIO.h)
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(configurator settings.cpp fileIO.cpp fileIO.h)
target_link_libraries( configurator ${OpenCV_LIBS} )
