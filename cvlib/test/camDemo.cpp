#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "opencv2/opencv.hpp"
#include <thread>
#include <stdio.h>
#include <string.h>
#include "library.h"
#include <X11/Xlib.h>
#include <sys/time.h>

//using namespace std;
using namespace cv;


int main(int argc, char** argv ) {
    region_detection rd;

    while(true){
        VideoCapture cap = VideoCapture(0);
        Mat frame;

        unsigned int counter = 0;
        cap >> frame;
        while(frame.data)
        {
            resize(frame, frame, Size(320,240),0,0);
            cap >> frame;
            counter++;

            rd.findRegion(frame);
        }
    }
}
