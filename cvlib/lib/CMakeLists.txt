find_package(OpenCV REQUIRED)
set(SOURCE_FILES library.cpp library.h camLib.h camLib.cpp character_detector.cpp character_detector.h)
set(RESOURCES_PATH "${PROJECT_SOURCE_DIR}/resources/")
        # configure a header file to pass some of the CMake settings
        # to the source code
configure_file ("${PROJECT_SOURCE_DIR}/lib/configuration.h.in" "${PROJECT_BINARY_DIR}/configuration.h")
        # add the binary tree to the search path for include files
        # so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(cvlib SHARED ${SOURCE_FILES} ../resources/trained_classifierNM1.xml ../resources/trained_classifierNM2.xml)
target_include_directories(cvlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cvlib ${OpenCV_LIBS})

