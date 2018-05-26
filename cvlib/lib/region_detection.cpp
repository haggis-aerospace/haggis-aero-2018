#include "region_detection.h"
#include <regex>
#include <vector>

using namespace std;
using namespace cv;

region_detection::region_detection(){ 
    initTrackbars();
    loadColourData();
    namedWindow("Region", 1);
        moveWindow("Region", 100, 100);
    namedWindow("Output", 1);
        moveWindow("Output", 100+320,100);
    namedWindow("White", 1);
        moveWindow("White", 100,100+240);
    namedWindow("Red", 1);
        moveWindow("Red", 100+320,100+240);

}
region_detection::~region_detection(){}


/**
 * @brief Filters mat of undesired colours
 * @param input
 * @param min
 * @param max
 * @param name
 * @return 
 */
Mat region_detection::filterImg(Mat input, Scalar min, Scalar max, string name)
{
    inRange(input, min, max, input);  //Filter out all colours except white/light grey
    
    std::vector<Mat> channels;
    split(input, channels);   //Splitting HSV space into 3 channels, using Hue for recognition
    
    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);
    
    if(name != ""){
        imshow(name.c_str(), Hue);
    	cv::waitKey(1);
    }
    return Hue;
}

/**
 * @brief Detects edges in Mat
 * @param src
 * @param blur
 * @return 
 */
int region_detection::detectEdge(Mat &src, bool blur)
{
    //if(blur)
    //    medianBlur(src, src, 11);
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
int region_detection::combineMat(Mat &src, Mat img2)
{
    cv::add(src, img2 , src);
    return 0;
}

/**
 * @brief Takes in mat, processes it to find region likely containing letter
 * @param src
 * @return 
 */
Mat region_detection::findRegion( Mat src )
{
    if(valueChanged)
        loadColourData();

    Mat letterRegion;

    if(!src.data){ cout << "No Data" << endl; return letterRegion; }


    Mat output(src.rows, src.cols, CV_8UC3, Scalar(0, 0, 0));
    Mat hsv;    //Conversion to HSV colour space
    cvtColor(src, hsv, CV_BGR2HSV);

    Mat redMat = filterImg(hsv, RED_DARK_MIN, RED_DARK_MAX, "");
    Mat LRedMat = filterImg(hsv, RED_LIGHT_MIN, RED_LIGHT_MAX, "");
    Mat whiteMat = filterImg(hsv, WHITE_MIN, WHITE_MAX, "");
    
    combineMat(redMat, LRedMat);
    
    medianBlur(redMat, redMat, 7);
    imshow("Red", redMat);
    cv::waitKey(1);

    medianBlur(whiteMat, whiteMat, 7);
    imshow("White", whiteMat);
    cv::waitKey(1);

    detectEdge(whiteMat, false);
    detectEdge(redMat, false);

    cv::bitwise_and(redMat, whiteMat, output);
    
    imshow("Output", output);
    cv::waitKey(1);

    //Retrieving image contours, used to determine ROI's
    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  
    
    findContours(output,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
    if(contoursH.size() <= 0){
        imshow("Region", src);
        cv::waitKey(1);
        return letterRegion;
    }
    
    //Find Max
    int maxIndex = 0;
    for(int i=0; i<contoursH.size(); i++)
        if(contoursH.at(i).size() >= contoursH.at(maxIndex).size())
            maxIndex = i;
    Rect region = boundingRect(contoursH.at(maxIndex));
    
    //Filter region smaller than 10% of total image
    if((double)region.width/(double)src.cols*100.0 < (double)REGION_MIN_PERCENTAGE_SIZE || (double)region.height/(double)src.rows*100.0 < (double)REGION_MIN_PERCENTAGE_SIZE){
        imshow("Region", src);
        cv::waitKey(1);
        return letterRegion;
    }
    
    RotatedRect rr = minAreaRect(contoursH.at(maxIndex));
    drawRotatedRect(src, rr);
    letterRegion = cropToRotatedRect(src, rr);
    imshow("Letter", letterRegion);
    imshow("Region", src);
    cv::waitKey(1);

    /*letterOut.letter = '~';
    letterOut.width = region.width;
    letterOut.height = region.height;
    letterOut.x = ((double)region.x+((double)region.width/2.0)) / (double)src.cols * 100.0;
    letterOut.y = ((double)region.y+((double)region.height/2.0)) / (double)src.rows * 100.0;
    */
    return letterRegion;
}


/**
 * @brief Draws a rotated rectangle onto a mat
 * @param src
 * @param rect
 */
void region_detection::drawRotatedRect(Mat src, RotatedRect rect)
{
    cv::Point2f rect_points[4];
    rect.points(rect_points);
    for( unsigned int i=0; i < 4; i++)
        cv::line(src, rect_points[i], rect_points[(i+1)%4], cv::Scalar(0,0,255),5);
}


/**
 * @brief Crops source image to a rotated rect
 * @return Cropped Mat
 */
Mat region_detection::cropToRotatedRect(Mat src, cv::RotatedRect rect)
{
    Mat output(src.rows, src.cols, CV_8UC3, Scalar(0, 0, 0));
    /*Rect ROI = rect.boundingRect();
    if(ROI.height > ROI.width){
        if(ROI.x + ROI.height > src.cols){
            ROI = Rect(ROI.x, ROI.y, src.cols - ROI.x, ROI.height );
        }else{
            ROI = Rect(ROI.x, ROI.y, ROI.height, ROI.height );
        }
    }else{
        if(ROI.y + ROI.width > src.rows){
            ROI = Rect(ROI.x, ROI.y, ROI.width, src.rows - ROI.y );
        }else{
            ROI = Rect(ROI.x, ROI.y, ROI.width, ROI.width );
        }
    }*/
        
    //Rect bounds(0,0,src.cols,src.rows);
    //src = src(ROI);
    
    Mat rotationMatrix = cv::getRotationMatrix2D(rect.center, rect.angle, 1);
    cv::warpAffine(src, output, rotationMatrix, output.size());
    rect.angle = 0;
    Rect ROI = rect.boundingRect();
    if(ROI.x < 0) ROI.x = 0;
    if(ROI.x > output.cols) ROI.x = output.cols - 1;
    if(ROI.x + ROI.width > output.cols) ROI.width = output.cols - ROI.x;
    if(ROI.y < 0) ROI.y = 0;
    if(ROI.y > output.rows) ROI.y = output.rows - 1;
    if(ROI.y + ROI.height > output.rows) ROI.height = output.rows - ROI.y;
    
    //rectangle(output, ROI, Scalar(255,0,0));
    //Rect bounds = Rect(0, 0, output.cols, output.rows);
    output = output(ROI);
    return output;
}

void region_detection::loadColourData()
{
    if(!valueChanged){
        colours.readData();
        valueChanged = true;
        loadColourData();
    }else{
        WHITE_MIN = cv::Scalar(colours.W_MIN_H, colours.W_MIN_S, colours.W_MIN_V);
        WHITE_MAX = cv::Scalar(colours.W_MAX_H, colours.W_MAX_S, colours.W_MAX_V);

        RED_DARK_MAX = cv::Scalar(colours.RD_MAX_H, colours.R_MAX_S, colours.R_MAX_V);
        RED_DARK_MIN = cv::Scalar(colours.RD_MIN_H, colours.R_MIN_S, colours.R_MIN_V);
        
        RED_LIGHT_MAX = cv::Scalar(colours.RL_MAX_H, colours.R_MAX_S, colours.R_MAX_V);
        RED_LIGHT_MIN = cv::Scalar(colours.RL_MIN_H, colours.R_MIN_S, colours.R_MIN_V);   
        valueChanged = false;
    }
}


/**
 * @function on_trackbar
 * @brief Callback for trackbar
 */
void on_trackbar( int, void* ){
        valueChanged = true;
        colours.saveData();
}


void initTrackbars()
{
    /// Initialize values
    valueChanged = false;
    
    namedWindow("Settings", 1);
    moveWindow("Settings", 100+320*2,100);
        
    /// Create Trackbars
    //WHITE
    char T_W_MIN_V[50]; //Trackbar White Min Value
    sprintf( T_W_MIN_V, "White Min V", 255);
    createTrackbar( T_W_MIN_V, "Settings", &colours.W_MIN_V, 255, on_trackbar );

    char T_W_MAX_S[50]; //Trackbar White Max Saturation
    sprintf( T_W_MAX_S, "White Max S", 255);
    createTrackbar( T_W_MAX_S, "Settings", &colours.W_MAX_S, 255, on_trackbar );

    //RED
    char T_RL_MIN_H[50]; //Trackbar Light Red Min Hue
    sprintf( T_RL_MIN_H, "Light Red Min H", 180);
    createTrackbar( T_RL_MIN_H, "Settings", &colours.RL_MIN_H, 180, on_trackbar );
    
    char T_RD_MAX_H[50]; //Trackbar Dark Red Max Hue
    sprintf( T_RD_MAX_H, "Dark Red Max H", 180);
    createTrackbar( T_RD_MAX_H, "Settings", &colours.RD_MAX_H, 180, on_trackbar );

    char T_R_MIN_S[50]; //Trackbar Red Min Saturation
    sprintf( T_R_MIN_S, "Red Min S", 180);
    createTrackbar( T_R_MIN_S, "Settings", &colours.R_MIN_S, 255, on_trackbar );

    char T_R_MIN_V[50]; //Trackbar Red Min Value
    sprintf( T_R_MIN_V, "Red Min V", 255);
    createTrackbar( T_R_MIN_V, "Settings", &colours.R_MIN_V, 255, on_trackbar );


    //Always maxed out
    /*
    char T_R_MAX_V[50]; //Trackbar Red Max Value
    sprintf( T_R_MAX_V, "Red Max V", 255);
    createTrackbar( T_R_MAX_V, "Settings", &colours.R_MAX_V, 255, on_trackbar );
    
    char T_W_MAX_H[50]; //Trackbar White Max Hue
    sprintf( T_W_MAX_H, "White Max H", 180 );
    createTrackbar( T_W_MAX_H, "Settings", &colours.W_MAX_H, 180, on_trackbar );
    
    char T_W_MIN_H[50]; //Trackbar White Min Hue
    sprintf( T_W_MIN_H, "White Min H", 180 );
    createTrackbar( T_W_MIN_H, "Settings", &colours.W_MIN_H, 180, on_trackbar );
    
    char T_W_MIN_S[50]; //Trackbar White Min Saturation
    sprintf( T_W_MIN_S, "White Min S", 180 );
    createTrackbar( T_W_MIN_S, "Settings", &colours.W_MIN_S, 255, on_trackbar );
    
    char T_W_MAX_V[50]; //Trackbar White Max Value
    sprintf( T_W_MAX_V, "White Max V", 255 );
    createTrackbar( T_W_MAX_V, "Settings", &colours.W_MAX_V, 255, on_trackbar );

    char T_RL_MAX_H[50]; //Trackbar Light Red Max Hue
    sprintf( T_RL_MAX_H, "Light Red Max H", 180 );
    createTrackbar( T_RL_MAX_H, "Settings", &colours.RL_MAX_H, 180, on_trackbar );
    
    char T_RD_MIN_H[50]; //Trackbar Dark Red Min Hue
    sprintf( T_RD_MIN_H, "Dark Red Min H", 180 );
    createTrackbar( T_RD_MIN_H, "Settings", &colours.RD_MIN_H, 180, on_trackbar );

    char T_R_MAX_S[50]; //Trackbar Red Max Saturation
    sprintf( T_R_MAX_S, "Red Max S", 255 );
    createTrackbar( T_R_MAX_S, "Settings", &colours.R_MAX_S, 255, on_trackbar );
    */
    
    on_trackbar(0,0);

}
