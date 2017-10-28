#include "camLib.h"
#include <regex>

using namespace std;
using namespace cv;
using namespace cv::text;

camLib::camLib()
{
    cap = cvCaptureFromCAM(0);  //Creating capture instance and initilizing OCRTesseract
    tess = OCRTesseract::create(NULL, NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 3, 10);
    timer = std::clock();   //For calculating FPS
}

camLib::~camLib()
{
    delete tess;
}

//Retrieving a new image from default webcam
Mat camLib::getImg()
{
    IplImage* newImg = cvQueryFrame(cap);   //Get new img, convert to Mat
    lastImg = cvarrToMat(newImg);
    return lastImg;
}



vector<cv::Mat> camLib::getBounds(Mat* img, bool display){ return getBounds(*img, display); }

//Returns array of Mats containing possible letters 
vector<cv::Mat> camLib::getBounds(Mat img, bool display)
{
    //Main configuration values
    Scalar WHITE_MIN(0  ,0 ,90 ); //initial white filter min/max HSV
    Scalar WHITE_MAX(180,65,255);
    int CONTOUR_SIZE_MIN = 400;   //Remove contours smaller than this
    double RATIO_THRESHOLD = .8;  //Removes contours with too high width:height ratio (value 0-1)
    
    Mat hsv;    //Conversion to HSV colour space
    cvtColor(img, hsv, CV_BGR2HSV);
    GaussianBlur(hsv, hsv, Size(15,15), 0, 0);  //Smooth image with blur
        
    inRange(hsv, WHITE_MIN, WHITE_MAX, hsv);  //Filter out all colours except white/light grey
    if(display) imshow("Filtered HSV", hsv);
    
    std::vector<Mat> channels;
    split(hsv, channels);   //Splitting HSV space into 3 channels, using Hue for recognition
    
    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);
    
    Canny(Hue, Hue, 180, 0);    //Get lines, thicken x4 for better detection    
    for(int i=0; i<4; ++i) 
        dilate(Hue, Hue, Mat());
    
    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  //Retrieving image contours, used to determine ROI's
    findContours(Hue,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Mat> output;
    // draw the contours to a copy of the input image:
    for( int i = 0; i< contoursH.size(); i++ ){
        if(contourArea(contoursH[i]) < CONTOUR_SIZE_MIN) continue; //Filter small contours
        if(hierarchyH[i][3] < 0) continue;  //Hollow out thick contours
        
        cv::Rect boundRect = boundingRect(contoursH[i]);    
        double ratio = boundRect.width/boundRect.height;
        if(ratio < 1-RATIO_THRESHOLD || ratio > 1+RATIO_THRESHOLD) continue;    //Filter rects too wide or tall
        
        Mat cropped = origHue(boundRect); //Crop to rect
        
        //Retrieve contour closest to centre, crop to it
        std::vector<std::vector<cv::Point>> contoursOut;
        std::vector<cv::Vec4i> hierarchyOut;
        findContours(cropped,contoursOut, hierarchyOut, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
        if(contoursOut.size() <= 0) continue;
        vector<Rect> croppedRects;
        for(int y=0;y<contoursOut.size();y++) croppedRects.push_back(boundingRect(contoursOut.at(y)));
        double min=10000;
        Rect target = croppedRects.at(0);
        Point centre(boundRect.width/2, boundRect.height/2);
        for(int y=0;y<croppedRects.size();y++)
        {
            double dist = cv::norm(Point(croppedRects.at(y).x + croppedRects.at(y).width/2
                ,croppedRects.at(y).y + croppedRects.at(y).height/2) - centre);
            if(dist < 0) dist = dist*-1;
            if(dist < min){ min = dist; target = croppedRects.at(y); }
        }
        cropped = cropped(target);
        
        //Add cropped img to output vector, draw rects onto img
        output.push_back(cropped);
        drawContours( img, contoursH, i, Scalar(0,0,255), 2, 8, hierarchyH, 0);
        rectangle( img, boundRect, Scalar(255,0,0));
    }
            
    //Show input/output images
    if(display){
        imshow("Output", img);
        imshow("Hue", Hue);
        imshow("InverseHue",Scalar::all(255) - origHue);
        waitKey(10);
    }
    
    //Calculate FPS
    double fps = ( std::clock() - timer ) / (double) CLOCKS_PER_SEC * 1000;
    fps = floor(fps);
    timer = std::clock();
    //printf("FPS: %.0f\n", fps);
    
    return output;
}


//Runs OCRTesseract on input image
vector<Letter> camLib::findLetter(Mat img, bool display, int min_confidence)
{
    vector<cv::Mat> regions = getBounds(img, display);  //Retrieve vector of Mats possibly containing letters
    vector<Letter> let;
    Mat scan;
    for(int i=0; i<regions.size(); i++) //Iterate through all retrieved Mats
    {
        if(regions[i].cols > 0){
            std::string res = "";
            regions[i].copyTo(scan);    //Apply white border area around Mat
            copyMakeBorder(scan, scan, 50,50,50,50,BORDER_CONSTANT,Scalar(0,0,0));
            scan = Scalar::all(255) - scan;     //Inverse colours to make letter black
            
            vector<Rect> *rects = new vector<Rect>();
            vector<std::string> *texts = new vector<std::string>();
            vector<float> *confidence = new vector<float>();
            tess->run(scan, res, rects, texts, confidence); //Runs Tesseract OCR on Mat
            
            //Ensure letter was found and filter low confidence
            if(!res.empty() && res != " " && confidence->at(0) >= min_confidence) 
            {
                printf("Size: %i,%i  Pos: %i,%i\n", rects->at(0).width, rects->at(0).height
                    ,rects->at(0).x, rects->at(0).y);
                printf("Confidence: %.2f: ", confidence->at(0));
                cout << "Found: " << res;
                let.push_back(Letter());    //Add letter to vector of type Letter
                let.at(let.size()-1).letter = res.c_str();
                
                if(display){imshow(("R"+to_string(i)),scan); waitKey(5);}
            }
            delete rects;   //Free up memory pointers
            delete texts;
            delete confidence;
        }
    }
    return let;
}


//Python Bindings
extern "C"{
    camLib* py_camLib() { return new camLib(); }
    Mat py_getImg(camLib* lib){ return lib->getImg(); }
    vector<Letter> py_findLetter(camLib* lib, Mat *img, bool display, int min_confidence){ return lib->findLetter(*img, display, min_confidence); }
}