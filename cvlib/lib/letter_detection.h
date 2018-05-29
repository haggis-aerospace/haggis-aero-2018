#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef LETTER_DETECTION_H
#define LETTER_DETECTION_H

using namespace std;
using namespace cv;

struct Letter {
    char letter = '~';  //Letter as found by tesseract
    int width = 0;    //Size of bounding box
    int height = 0;
    int x = 0;      //Coords of letter
    int y = 0;
};


class letter_detection
{
    
    
public:
    
    Letter findLetter(cv::Mat region);
    
    
    letter_detection();
    ~letter_detection();

};

#endif // LETTER_DETECTION_H
