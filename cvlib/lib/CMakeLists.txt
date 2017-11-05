set(SOURCE_FILES library.cpp library.h camLib.h camLib.cpp character_detector.cpp character_detector.h)

add_library(cvlib SHARED ${SOURCE_FILES})
target_include_directories(cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cvlib ${OpenCV_LIBS} ${FastText_LIBS})

