#include "library.h"

extern "C"
{
void hello() {
    std::cout << "Hello, World!" << std::endl;
}
void libtest() {
    std::cout << cv::getBuildInformation();
}
}
