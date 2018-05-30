#include "letter_detection.h"

//using namespace std;
//using namespace cv;

letter_detection::letter_detection()
{
    //if (api->Init(NULL, "eng"))
    //{
            //std::cout << "Error Initilizing Tesseract" << std::endl;
            //exit(1);
    //}
}

letter_detection::~letter_detection()
{
}

/*Letter letter_detection::findLetter(cv::Mat src, int *accuracy)
{
    Letter letter;
    
    if(!src.data){ std::cout << "letter_detection: Invalid Input Data" << std::endl;
        return letter; }
    
    cv::Mat inputImg = src.clone();
    
    //Convert img to greyscale, invert colour
    cv::cvtColor(inputImg, inputImg, CV_BGR2GRAY);
    cv::bitwise_not(src,src);
    
    
    cv::imshow("Letter Detection", inputImg);
    cv::waitKey(1);
    
    return letter;
}*/