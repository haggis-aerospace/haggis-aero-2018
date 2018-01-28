#include "camLib.h"
#include <regex>
#include <vector>

using namespace std;
using namespace cv;

camLib::camLib(){ loadColourData(); }
camLib::~camLib(){}


/**
 * @brief Filters mat of undesired colours
 * @param input
 * @param min
 * @param max
 * @param name
 * @return 
 */
Mat camLib::filterImg(Mat input, Scalar min, Scalar max, string name)
{
    inRange(input, min, max, input);  //Filter out all colours except white/light grey
    
    std::vector<Mat> channels;
    split(input, channels);   //Splitting HSV space into 3 channels, using Hue for recognition
    
    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);
    
    if(name != "")
        imshow(name.c_str(), Hue);
    return Hue;
}

/**
 * @brief Detects edges in Mat
 * @param src
 * @param blur
 * @return 
 */
int camLib::detectEdge(Mat &src, bool blur)
{
    if(blur)
        medianBlur(src, src, 11);
    Canny(src,src, 180, 0);    //Get lines, thicken x4 for better detection    
    for(int i=0; i<8; ++i) 
        dilate(src, src, Mat());
    return 0;
}

/**
 * @brief Combines 2 mats into single output
 * @param src
 * @param img2
 * @return 
 */
int camLib::combineMat(Mat &src, Mat img2)
{
    cv::add(src, img2 , src);
    return 0;
}

/**
 * @brief Takes in mat, processes it to find region likely containing letter
 * @param src
 * @return 
 */
Letter camLib::findLetter( Mat src )
{
    Letter letterOut;
    if(!src.data){ return letterOut; }
    Mat output(src.rows, src.cols, CV_8UC3, Scalar(0, 0, 0));
    medianBlur(src, src, 15);
    Mat hsv;    //Conversion to HSV colour space
    cvtColor(src, hsv, CV_BGR2HSV);
    //FilterImg is run asyncronously to process 3 mats at once
    vector<thread> threads;
    future<Mat> f_DRM = async(&camLib::filterImg, this, hsv, RED_DARK_MIN, RED_DARK_MAX, "");
    future<Mat> f_LRM = async(&camLib::filterImg, this, hsv, RED_LIGHT_MIN, RED_LIGHT_MAX, "");
    future<Mat> f_WM  = async(&camLib::filterImg, this, hsv, WHITE_MIN, WHITE_MAX, "White");
    
    Mat redMat = f_DRM.get();
    Mat LRedMat = f_LRM.get();        
    combineMat(redMat, LRedMat);
    imshow("Red", redMat);
    Mat whiteMat = f_WM.get();
    
    future<int> f_d_RM = async(&camLib::detectEdge, this, std::ref(whiteMat), true);
    future<int> f_d_WM = async(&camLib::detectEdge, this, std::ref(redMat), true);
    f_d_RM.get();
    f_d_WM.get();
    cv::bitwise_and(redMat, whiteMat, output);
    
    imshow("Output", output);
    imshow("Region", src);
    cv::waitKey(5);
    
    //Retrieving image contours, used to determine ROI's
    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  
    findContours(output,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
    
    if(contoursH.size() <= 0)
        return letterOut;
    
    //Find Max
    int maxIndex = 0;
    for(int i=0; i<contoursH.size(); i++)
        if(contoursH.at(i).size() >= contoursH.at(maxIndex).size())
            maxIndex = i;
    Rect region = boundingRect(contoursH.at(maxIndex));
    
    //Filter region smaller than 10% of total image
    //cout << region.width/src.cols*100 << ", " << region.height/src.rows*100 << endl;
    if((double)region.width/(double)src.cols*100.0 < (double)REGION_MIN_PERCENTAGE_SIZE || (double)region.height/(double)src.rows*100.0 < (double)REGION_MIN_PERCENTAGE_SIZE)
        return letterOut;
        
    rectangle(src, boundingRect(contoursH.at(maxIndex)), Scalar(0,0,255),5);    
    imshow("Region", src);
    letterOut.letter = '~';
    letterOut.width = region.width;
    letterOut.height = region.height;
    letterOut.x = ((double)region.x+((double)region.width/2.0)) / (double)src.cols * 100.0;
    letterOut.y = ((double)region.y+((double)region.height/2.0)) / (double)src.rows * 100.0;
    cv::waitKey(5);
    
    return letterOut;
}


void camLib::loadColourData()
{
    colours.readData();

    WHITE_MIN = cv::Scalar(colours.W_MIN_H, colours.W_MIN_S, colours.W_MIN_V);
    WHITE_MAX = cv::Scalar(colours.W_MAX_H, colours.W_MAX_S, colours.W_MAX_V);

    RED_DARK_MAX = cv::Scalar(colours.RD_MAX_H, colours.R_MAX_S, colours.R_MAX_V);
    RED_DARK_MIN = cv::Scalar(colours.RD_MIN_H, colours.R_MIN_S, colours.R_MIN_V);
    
    RED_LIGHT_MAX = cv::Scalar(colours.RL_MAX_H, colours.R_MAX_S, colours.R_MAX_V);
    RED_LIGHT_MIN = cv::Scalar(colours.RL_MIN_H, colours.R_MIN_S, colours.R_MIN_V);
}