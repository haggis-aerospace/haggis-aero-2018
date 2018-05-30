cmake_minimum_required(VERSION 2.8)
project( videoDownlink )
find_package( OpenCV REQUIRED )

add_executable( server Server.cpp Server.h PracticalSocket.cpp )
target_link_libraries( server ${OpenCV_LIBS} )

add_executable( client Client.cpp PracticalSocket.cpp )
target_link_libraries( client LINK_PUBLIC X11 cvlib ${OpenCV_LIBS})
