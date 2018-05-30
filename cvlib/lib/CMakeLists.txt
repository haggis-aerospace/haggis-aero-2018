find_package( OpenCV REQUIRED )
find_package( PkgConfig REQUIRED )
pkg_search_module( TESSERACT REQUIRED tesseract )
pkg_search_module( LEPTONICA REQUIRED lept )

set(SOURCE_FILES fileIO.cpp fileIO.h library.h region_detection.h region_detection.cpp letter_detection.h letter_detection.cpp)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${TESSERACT_INCLUDE_DIRS} )
include_directories( ${LEPTONICA_INCLUDE_DIRS} )

add_library(cvlib SHARED ${SOURCE_FILES})

target_include_directories (cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (cvlib PUBLIC ${Tesseract_INCLUDE_DIRS})

target_link_libraries( cvlib ${OpenCV_LIBS} )
target_link_libraries( cvlib ${Tesseract_LIBRARIES})
