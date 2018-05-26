find_package( OpenCV REQUIRED )
set(SOURCE_FILES fileIO.cpp fileIO.h library.h region_detection.h region_detection.cpp)
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library(cvlib SHARED ${SOURCE_FILES})
target_include_directories (cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries( cvlib ${OpenCV_LIBS} )
