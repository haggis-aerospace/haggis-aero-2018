#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <stddef.h>
#include <ctime>
#include <mutex>
#include <thread>
#include <time.h>
#include <chrono>
#include "settings.h"
#include <future>

#ifndef CAMLIB_H
#define CAMLIB_H

using namespace cv;
using namespace std;

struct Letter {
    char letter = '~';
    int width = 0;    //Size of bounding box
    int height = 0;
    int x = 0;
    int y = 0;
    int pos = 0;     //Horizontal position as % from left of image
    int avSize = 0; //Average size
};


class camLib
{
private:
    //Main configuration values
    cv::Scalar WHITE_MIN;
    cv::Scalar WHITE_MAX;
    cv::Scalar RED_LIGHT_MIN;
    cv::Scalar RED_LIGHT_MAX;
    cv::Scalar RED_DARK_MIN;
    cv::Scalar RED_DARK_MAX;
    colourData colours;
    const int REGION_MIN_PERCENTAGE_SIZE = 10;
    
    int combineMat(Mat &src, Mat img2);
    int detectEdge(Mat &src, bool blur=true);
    Mat filterImg(Mat input, Scalar min, Scalar max, string name="");

public:
    void loadColourData();
    Letter findLetter( Mat src );
    camLib();
    ~camLib();
};

#endif // CAMLIB_H
