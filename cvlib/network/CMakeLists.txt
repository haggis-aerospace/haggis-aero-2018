cmake_minimum_required(VERSION 2.8)
project( videoDownlink )
find_package( OpenCV REQUIRED )


######### Server 

add_executable( server Server.cpp Server.h PracticalSocket.cpp )
target_link_libraries( server ${OpenCV_LIBS} )


######### Client

add_executable( client Client.cpp PracticalSocket.cpp )
target_link_libraries( client LINK_PUBLIC X11 cvlib ${OpenCV_LIBS})
target_link_libraries( client LINK_PUBLIC tesseract)
