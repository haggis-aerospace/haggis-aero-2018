#include "library.h"
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "opencv2/opencv.hpp"
#include <thread>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <sys/time.h>

using namespace std;
using namespace cv;


int main(int argc, char** argv ) {

    if(argc < 2){
      cout << "Usage: ./recordedDemo <fileName> [startFrame] [endFrame]" << endl;
      return 0;
    }

    region_detection rd;

    int startFrame = 0;
    int endFrame = -1;

    if(argc >= 3)
      startFrame = atoi(argv[2]);
    if(argc >= 4)
      endFrame = atoi(argv[3]);


    while(true){
        VideoCapture cap = VideoCapture(argv[1]);
        Mat frame;
        cap.set(1,startFrame);

        int fps = cap.get(CV_CAP_PROP_FPS);

        unsigned int counter = startFrame++;
        cap >> frame;

        while(frame.data)
        {
            clock_t last_cycle = clock();

            cap >> frame;
            resize(frame, frame, Size(320,240),0,0);
            counter++;
            cout << "Frame: " << counter << endl;
            rd.findRegion(frame);

            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / CLOCKS_PER_SEC * 1000;
            usleep(((1000000/fps) - duration*1000));
        }
    }
}
