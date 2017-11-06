<<<<<<< HEAD
find_package( OpenCV REQUIRED )
set(SOURCE_FILES library.cpp library.h OCRDemo.h scene.h)
include_directories( ${OpenCV_INCLUDE_DIRS} )
=======
set(SOURCE_FILES library.cpp library.h camLib.h camLib.cpp character_detector.cpp character_detector.h)

set(RESOURCES_PATH "${PROJECT_SOURCE_DIR}/resources/")

configure_file(
        "${PROJECT_SOURCE_DIR}/lib/configuration.h.in"
        "${PROJECT_BINARY_DIR}/lib/configuration.h"
)

include_directories("${PROJECT_BINARY_DIR}")
>>>>>>> SaneOCR

add_library(cvlib SHARED ${SOURCE_FILES})
target_include_directories(cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cvlib ${OpenCV_LIBS} ${FastText_LIBS})