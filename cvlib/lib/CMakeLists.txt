find_package( OpenCV REQUIRED )
set(SOURCE_FILES library.cpp library.h)
add_library(cvlib SHARED ${SOURCE_FILES})
target_include_directories (cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
