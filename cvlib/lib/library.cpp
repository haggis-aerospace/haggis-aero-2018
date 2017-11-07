#include "library.h"

#include <iostream>
#include <opencv2/opencv.hpp>

void hello() {
        std::cout << "Hello, World!" << std::endl;
    }
    void libtest() {
         std::cout << cv::getBuildInformation();
    }
