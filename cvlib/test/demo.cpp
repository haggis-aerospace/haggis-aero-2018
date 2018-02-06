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

using namespace std;
using namespace cv;

using namespace std;


int main(int argc, char** argv ) {
    camLib library;
    
    while(true){
        VideoCapture cap("downStream.avi");
        Mat frame;
        for(int i=0; i<1000; i++)
            cap.grab();
        cap >> frame;
        while(frame.data)
        {
            cap >> frame;
            library.findLetter(frame);
        }
    }
}
