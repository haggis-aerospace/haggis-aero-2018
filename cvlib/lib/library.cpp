#include "library.h"

#include <iostream>
#include <opencv2/opencv.hpp>

extern "C"
{
    void hello() {
        std::cout << "Hello, World!" << std::endl;
    }
    void libtest() {
         std::cout << cv::getBuildInformation();
    }
}
