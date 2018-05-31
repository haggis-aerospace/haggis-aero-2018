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
    Mat cropToRotatedRect(Mat src, RotatedRect rect);
    
public:
    void loadColourData();
    Mat filterImg(Mat input, Scalar min, Scalar max, string name="");
    Mat findRegion( Mat src, std::pair<int,int> *coords = 0);
    region_detection();
    ~region_detection();
};


bool valueChanged = false;    
void on_trackbar( int, void* );
void initTrackbars();

#endif // region_detection_H
