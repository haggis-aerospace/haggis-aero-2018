#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <opencv2/text.hpp>
#include <stddef.h>
#include "opencv2/text/ocr.hpp"
#include <ctime>
#include <mutex>
#include <thread>
#include <time.h>
#include <chrono>
#include <opencv2/tracking.hpp>

#ifndef CAMLIB_H
#define CAMLIB_H

using namespace cv;
using namespace cv::text;


struct Letter {
    char letter; //Letter as recognised by tesseract
    int width;    //Size of bounding box
    int height;
    int x;
    int y;
    int pos;     //Horizontal position as % from left of image
    int avSize; //Average size
};


class camLib
{
private:
    //Main configuration values
    cv::Scalar WHITE_MIN = cv::Scalar(0,0,90); //initial white filter min/max HSV
    cv::Scalar WHITE_MAX = cv::Scalar(180,80,255);
    const int CONTOUR_SIZE_MIN = 200;   //Remove contours smaller than this
    const float RATIO_THRESHOLD = .4;  //Removes contours with too high width:height ratio (value 0-1)
    const int LETTER_MIN_WIDTH = 7;
    const int LETTER_MIN_HEIGHT = 10;
    const int CAP_WIDTH = 300;
    const int CAP_HEIGHT = 225;
    const int CERTAINTY_PERCENTAGE = 80;
    
    //Values for tracking 
    const int HISTORY_SIZE = 6;    //Num of letters to store in history
    const int MAX_SCAN_GAP = 1000;  //Max delay between detecting (not tracking)
    const int MAX_HISTORY_AGE = 10000; //Max age of relevant data in history
    
    bool display;
    
    Ptr<OCRTesseract> tess;
    VideoCapture cap;
    Mat lastImg;
    unsigned long lastFrameScan;
    cv::Ptr<cv::Tracker> tracker;
    bool tracking = false;
    Letter trackedLetter;
    bool checkRegion(cv::Mat input, std::vector<std::vector<cv::Point>> contours, int index, std::vector<cv::Vec4i> hierarchy, std::vector<std::pair<cv::Mat,cv::Point>> &output);
    void updateHistory(std::vector<Letter> letters);
    std::deque<std::vector<std::pair<Letter,unsigned long>>> history;
    
public:
    camLib(bool display=true);
    ~camLib();
    
    Mat getImg();
    std::vector<Letter> findLetter(Mat img, int min_confidence=70);
    std::vector<Letter> findLetter(int min_confidence=70){ return findLetter(getImg(), min_confidence); }

    std::vector< std::pair<cv::Mat, cv::Point>> getBounds(Mat img);
    std::vector< std::pair<cv::Mat, cv::Point>> getBounds(Mat* img);
    
    Letter mostOccouring();
    
    bool tryTrack(cv::Mat &img);
    bool isTracking(){ return tracking; }
};


static unsigned long currentTime(){
    unsigned long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

//Python Bindings
extern "C"{
    camLib* py_camLib(bool disp = false);
    Mat py_getImg(camLib* lib);
    void py_findLetter(camLib* lib, int min_confidence=70);
    Letter py_mostOccouring(camLib* lib);
}

#endif // CAMLIB_H
