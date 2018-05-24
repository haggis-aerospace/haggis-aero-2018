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

using namespace std;
using namespace cv;


int main(int argc, char** argv ) {
    camLib library;
    
    while(true){
        VideoCapture cap = VideoCapture(1);
        Mat frame;
        //for(int i=0; i<1000; i++)
        //    cap.grab();
        //cap >> frame;
        //clock_t last_cycle = clock();
        unsigned int counter = 0;
        cap >> frame;
        while(frame.data)
        {
            resize(frame, frame, Size(320,240),0,0);
            cap >> frame;
            counter++;
        //    cout << "Frame: " << counter << endl;
            library.findLetter(frame);
            
       //     clock_t next_cycle = clock();
       //     double duration = (next_cycle - last_cycle) / CLOCKS_PER_SEC;
       //     usleep((70000 - duration*1000));
        }
    }
}
