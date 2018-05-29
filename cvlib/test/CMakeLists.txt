add_executable (camDemo camDemo.cpp)
add_executable (recordedDemo recordedDemo.cpp)
target_link_libraries (camDemo LINK_PUBLIC cvlib)
target_link_libraries (recordedDemo LINK_PUBLIC cvlib)
