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
#include "fileIO.h"
#include <future>

#ifndef REGION_DETECTION_H
#define REGION_DETECTION_H

using namespace cv;
using namespace std;

colourData colours;


struct Letter {
    char letter = '~';
    int width = 0;    //Size of bounding box
    int height = 0;
    int x = 0;
    int y = 0;
    int pos = 0;     //Horizontal position as % from left of image
    int avSize = 0; //Average size
};


class region_detection
{
private:
    //Main configuration values
    cv::Scalar WHITE_MIN;
    cv::Scalar WHITE_MAX;
    cv::Scalar RED_LIGHT_MIN;
    cv::Scalar RED_LIGHT_MAX;
    cv::Scalar RED_DARK_MIN;
    cv::Scalar RED_DARK_MAX;
    const int REGION_MIN_PERCENTAGE_SIZE = 5;
    int blurSize;
    
    void drawRotatedRect(Mat src, RotatedRect rect);
    int combineMat(Mat &src, Mat img2);
    int detectEdge(Mat &src, bool blur=true);
    Mat filterImg(Mat input, Scalar min, Scalar max, string name="");

    
public:
    void loadColourData();
    Mat findRegion( Mat src );
    region_detection();
    ~region_detection();
};


bool valueChanged = false;    
void on_trackbar( int, void* );
void initTrackbars();

#endif // region_detection_H
