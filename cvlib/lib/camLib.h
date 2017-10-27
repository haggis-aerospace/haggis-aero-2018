#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <opencv2/text.hpp>
#include <stddef.h>
#include "opencv2/text/ocr.hpp"
#include <ctime>

#ifndef CAMLIB_H
#define CAMLIB_H

using namespace cv;
using namespace cv::text;


struct Letter {
    const char* letter; //Letter as recognised by tesseract
    int size;    //Size of bounding box
    int pos;     //Horizontal position as % from left of image
};


class camLib
{
private:
    Ptr<OCRTesseract> tess;
    CvCapture* cap;
    Mat lastImg;
    std::clock_t timer;
public:
    camLib();
    ~camLib();
    
    Mat getImg();
    std::vector<Letter> findLetter(Mat img, bool display=false, int min_confidence=70);

    std::vector<cv::Mat> getBounds(Mat img, bool display=false);
    std::vector<cv::Mat> getBounds(Mat* img, bool display=false);
};


//Python Bindings
extern "C"{
    camLib* py_camLib();
    Mat py_getImg(camLib* lib);
    std::vector<Letter> py_findLetter(camLib* lib, Mat *img, bool display=false, int min_confidence=70);
}

#endif // CAMLIB_H
