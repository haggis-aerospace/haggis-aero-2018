#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "fileIO.h"
#include <unistd.h>
#include <future>

#define THREADED 0

using namespace cv;
using namespace std;

/// Global Variables
colourData data;

cv::Scalar WHITE_MIN;
cv::Scalar WHITE_MAX;
cv::Scalar RED_LIGHT_MIN;
cv::Scalar RED_LIGHT_MAX;
cv::Scalar RED_DARK_MIN;
cv::Scalar RED_DARK_MAX;


int blurSize;
bool valueChanged;    
Mat src;


/**
 * @function on_trackbar
 * @brief Callback for trackbar
 */
void on_trackbar( int, void* ){
        WHITE_MIN = cv::Scalar(data.W_MIN_H, data.W_MIN_S, data.W_MIN_V);
        WHITE_MAX = cv::Scalar(data.W_MAX_H, data.W_MAX_S, data.W_MAX_V);
        
        RED_DARK_MAX = cv::Scalar(data.RD_MAX_H, data.R_MAX_S, data.R_MAX_V);
        RED_DARK_MIN = cv::Scalar(data.RD_MIN_H, data.R_MIN_S, data.R_MIN_V);
        
        RED_LIGHT_MAX = cv::Scalar(data.RL_MAX_H, data.R_MAX_S, data.R_MAX_V);
        RED_LIGHT_MIN = cv::Scalar(data.RL_MIN_H, data.R_MIN_S, data.R_MIN_V);
        valueChanged = true;
}


void initTrackbars()
{
    /// Initialize values
    blurSize = 0;
    valueChanged = false;
    
    /// Create Trackbars
    //WHITE
    char T_W_MIN_V[50]; //Trackbar White Min Value
    sprintf( T_W_MIN_V, "White Min V", 255);
    createTrackbar( T_W_MIN_V, "Settings", &data.W_MIN_V, 255, on_trackbar );

    char T_W_MAX_S[50]; //Trackbar White Max Saturation
    sprintf( T_W_MAX_S, "White Max S", 255);
    createTrackbar( T_W_MAX_S, "Settings", &data.W_MAX_S, 255, on_trackbar );

    //RED
    char T_RL_MIN_H[50]; //Trackbar Light Red Min Hue
    sprintf( T_RL_MIN_H, "Light Red Min H", 180);
    createTrackbar( T_RL_MIN_H, "Settings", &data.RL_MIN_H, 180, on_trackbar );
    
    char T_RD_MAX_H[50]; //Trackbar Dark Red Max Hue
    sprintf( T_RD_MAX_H, "Dark Red Max H", 180);
    createTrackbar( T_RD_MAX_H, "Settings", &data.RD_MAX_H, 180, on_trackbar );

    char T_R_MIN_S[50]; //Trackbar Red Min Saturation
    sprintf( T_R_MIN_S, "Red Min S", 180);
    createTrackbar( T_R_MIN_S, "Settings", &data.R_MIN_S, 255, on_trackbar );

    char T_R_MIN_V[50]; //Trackbar Red Min Value
    sprintf( T_R_MIN_V, "Red Min V", 255);
    createTrackbar( T_R_MIN_V, "Settings", &data.R_MIN_V, 255, on_trackbar );


    //Always maxed out
    /*
    char T_R_MAX_V[50]; //Trackbar Red Max Value
    sprintf( T_R_MAX_V, "Red Max V", 255);
    createTrackbar( T_R_MAX_V, "Settings", &data.R_MAX_V, 255, on_trackbar );
    
    char T_W_MAX_H[50]; //Trackbar White Max Hue
    sprintf( T_W_MAX_H, "White Max H", 180 );
    createTrackbar( T_W_MAX_H, "Settings", &data.W_MAX_H, 180, on_trackbar );
    
    char T_W_MIN_H[50]; //Trackbar White Min Hue
    sprintf( T_W_MIN_H, "White Min H", 180 );
    createTrackbar( T_W_MIN_H, "Settings", &data.W_MIN_H, 180, on_trackbar );
    
    char T_W_MIN_S[50]; //Trackbar White Min Saturation
    sprintf( T_W_MIN_S, "White Min S", 180 );
    createTrackbar( T_W_MIN_S, "Settings", &data.W_MIN_S, 255, on_trackbar );
    
    char T_W_MAX_V[50]; //Trackbar White Max Value
    sprintf( T_W_MAX_V, "White Max V", 255 );
    createTrackbar( T_W_MAX_V, "Settings", &data.W_MAX_V, 255, on_trackbar );

    char T_RL_MAX_H[50]; //Trackbar Light Red Max Hue
    sprintf( T_RL_MAX_H, "Light Red Max H", 180 );
    createTrackbar( T_RL_MAX_H, "Settings", &data.RL_MAX_H, 180, on_trackbar );
    
    char T_RD_MIN_H[50]; //Trackbar Dark Red Min Hue
    sprintf( T_RD_MIN_H, "Dark Red Min H", 180 );
    createTrackbar( T_RD_MIN_H, "Settings", &data.RD_MIN_H, 180, on_trackbar );

    char T_R_MAX_S[50]; //Trackbar Red Max Saturation
    sprintf( T_R_MAX_S, "Red Max S", 255 );
    createTrackbar( T_R_MAX_S, "Settings", &data.R_MAX_S, 255, on_trackbar );
    */
    
    on_trackbar(0,0);

}


Mat filterImg(Mat input, Scalar min, Scalar max, string name="")
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

int detectEdge(Mat &src, bool blur=true)
{
    if(blur)
        medianBlur(src, src, 11);
    Canny(src,src, 180, 0);    //Get lines, thicken x4 for better detection    
    for(int i=0; i<8; ++i) 
        dilate(src, src, Mat());
    return 0;
}

int combineMat(Mat &src, Mat img2)
{
    cv::add(src, img2 , src);
    return 0;
}

int main( int argc, char** argv )
{
    
    VideoCapture cap = VideoCapture(1);

    /// Create Windows
    namedWindow("Settings", 1);

    initTrackbars();
    
    
    std::deque<double> frames;  //For calculating FPS
    std::clock_t timer = std::clock();   
    
    while(true){
    
        /// Read image ( same size, same type )
        cap.read(src);
        if( !src.data ) { printf("Error reading camera \n"); continue; }
        resize(src, src, Size(350,300)); 

        Mat output(src.rows, src.cols, CV_8UC3, Scalar(0, 0, 0));
        
        medianBlur(src, src, 15);
        
        Mat hsv;    //Conversion to HSV colour space
        cvtColor(src, hsv, CV_BGR2HSV);
        
        Mat redMat;
        Mat whiteMat;
        
        if(THREADED){
            vector<thread> threads;
            future<Mat> f_DRM = async(filterImg,hsv, RED_DARK_MIN, RED_DARK_MAX, "");
            future<Mat> f_LRM = async(filterImg,hsv, RED_LIGHT_MIN, RED_LIGHT_MAX, "");
            future<Mat> f_WM  = async(filterImg,hsv, WHITE_MIN, WHITE_MAX, "White");

            redMat = f_DRM.get();
            Mat LRedMat = f_LRM.get();        
            combineMat(redMat, LRedMat);                

            whiteMat = f_WM.get();

            future<int> f_d_RM = async(detectEdge,std::ref(whiteMat), true);
            future<int> f_d_WM = async(detectEdge,std::ref(redMat), true);
            f_d_RM.get();
            f_d_WM.get();
        }else{
            redMat = filterImg(hsv, RED_DARK_MIN, RED_DARK_MAX, "");
            Mat LRedMat = filterImg(hsv, RED_LIGHT_MIN, RED_LIGHT_MAX, "");
            whiteMat = filterImg(hsv, WHITE_MIN, WHITE_MAX, "White");
            
            combineMat(redMat, LRedMat);
            
            detectEdge(whiteMat, true);
            detectEdge(redMat, true);
        }
        imshow("Red", redMat);
        cv::bitwise_and(redMat, whiteMat, output);
        
        imshow("Output", output);
        
        std::vector<std::vector<cv::Point>> contoursH;
        std::vector<cv::Vec4i> hierarchyH;  //Retrieving image contours, used to determine ROI's
        findContours(output,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
        
        //Find Max
        /*int maxIndex = 0;
        for(int i=0; i<contoursH.size(); i++)
            if(contoursH.at(i).size() >= contoursH.at(maxIndex).size())
                maxIndex = i;
        
        drawContours(output, contoursH, maxIndex, Scalar(180,100,100));
        */
        
        for(int i=0; i<contoursH.size(); i++){
            if(contourArea(contoursH[i]) < 40) continue;
            //drawContours(src, contoursH, i, Scalar(180,100,100), 10);
            rectangle(src, boundingRect(contoursH[i]), Scalar(0,0,255),5);
        }
        
        if(valueChanged)
        {
                valueChanged = false;
                data.saveData();
        }
        
        
        //Calculate FPS
        double fps = ( std::clock() - timer ) / (double) CLOCKS_PER_SEC * 100;
        frames.push_front(60.0/fps);
        if(frames.size() > 4)
            frames.pop_back();
        double totalfps = 0;
        for(int i=0; i<frames.size(); i++)
            totalfps = totalfps + frames[i];
        fps = totalfps/frames.size();
        
        ostringstream strs;
        strs << fps;
        string strFPS = strs.str();


        //printf("FPS: %.3f\n", fps);
        putText(src, "FPS: " + strFPS, cvPoint(30,30), 
            FONT_HERSHEY_COMPLEX_SMALL, 1, cvScalar(0,255,0), 1, CV_AA);
        timer = std::clock();
        
        imshow("Contours", src);

        cv::waitKey(5);
    }
    return 0;
}
