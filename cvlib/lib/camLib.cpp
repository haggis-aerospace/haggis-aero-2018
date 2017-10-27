#include "camLib.h"
#include <regex>

using namespace std;
using namespace cv;
using namespace cv::text;

camLib::camLib()
{
    cap = cvCaptureFromCAM(0);
    tess = OCRTesseract::create(NULL, NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", tesseract::OEM_DEFAULT, tesseract::PSM_SINGLE_CHAR);
}

camLib::~camLib()
{
    delete tess;
}

//Retrieving a new image from default webcam
Mat camLib::getImg()
{
    IplImage* newImg = cvQueryFrame(cap);
    lastImg = cvarrToMat(newImg);
    //resize(lastImg, lastImg, Size(300,210));
    return lastImg;
}



//vector<cv::Mat> camLib::getBounds(Mat* img){ return getBounds(*img, false); }
vector<cv::Mat> camLib::getBounds(Mat* img, bool display){ return getBounds(*img, display); }
//vector<cv::Mat> camLib::getBounds(Mat img){ return getBounds(img, false); }

//Returns array of Mats containing possible letters 
vector<cv::Mat> camLib::getBounds(Mat img, bool display)
{
    Mat hsv;
    cvtColor(img, hsv, CV_BGR2HSV);
    GaussianBlur(hsv, hsv, Size(15,15), 0, 0);
        
    inRange(hsv, Scalar(0, 0, 90), Scalar(180, 65, 255), hsv);
    if(display) imshow("Filtered HSV", hsv);

    std::vector<Mat> channels;
    split(hsv, channels);
    
    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);
    int shift = 14; //Shifting hue channel by 28°
    for(int j=0; j<Hue.rows; ++j)
        for(int i=0; i<Hue.cols; ++i)
            Hue.at<unsigned char>(j,i) = (Hue.at<unsigned char>(j,i) + shift)%180;
    channels[0] = Hue;
    
    if(display) imshow("PreHue", origHue);
    Canny(Hue, Hue, 180, 0);
    
    for(int i=0; i<4; ++i)
        dilate(Hue, Hue, Mat());
    
    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;
    findContours(Hue,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Mat> output(contoursH.size());
    // draw the contours to a copy of the input image:
    for( int i = 0; i< contoursH.size(); i++ ){
        if(contourArea(contoursH[i]) < 400) continue;
        if(hierarchyH[i][3] < 0) continue;
        cv::Rect boundRect = boundingRect(contoursH[i]);
        Mat cropped = origHue(boundRect);
        output.push_back(cropped);
        drawContours( img, contoursH, i, Scalar(0,0,255), 2, 8, hierarchyH, 0);
        rectangle( img, boundRect, Scalar(255,0,0));
    }
        
    if(display){
        imshow("Output", img);
        imshow("Hue", Hue);
        imshow("InverseHue",Scalar::all(255) - origHue);
        waitKey(10);
    }
    
    return output;
}

Letter camLib::findLetter(Mat img, bool display, int min_confidence)
{
    vector<cv::Mat> regions = getBounds(img, display);
    Letter let;
    let.letter = "";
    Mat scan;
    for(int i=0; i<regions.size(); i++)
    {
        if(regions[i].cols > 0){
            std::string res = "";
            regions[i].copyTo(scan);
            copyMakeBorder(scan, scan, 50,50,50,50,BORDER_CONSTANT,Scalar(0,0,0));
            scan = Scalar::all(255) - scan;
            vector<Rect> *rects = new vector<Rect>();
            vector<std::string> *texts = new vector<std::string>();
            vector<float> *confidence = new vector<float>();
            tess->run(scan, res, rects, texts, confidence);
            if(!res.empty() && res != " " && confidence->at(0) >= 70)
            {
                cout << rects->at(0) << endl;
                printf("Confidence: %.2f: ", confidence->at(0));
                cout << "Found: " << res;
                let.letter = res.c_str();
            }
            delete rects;
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
    Letter py_findLetter(camLib* lib, Mat *img, bool display, int min_confidence){ return lib->findLetter(*img, display, min_confidence); }
}