#include "camLib.h"
#include <regex>

using namespace std;
using namespace cv;
using namespace cv::text;

camLib::camLib() {
    cap = cvCaptureFromCAM(0);  //Creating capture instance and initilizing OCRTesseract
    tess = OCRTesseract::create(NULL, NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 3, 10);
    timer = std::clock();   //For calculating FPS
}

camLib::~camLib() {
    delete tess;
}

//Retrieving a new image from default webcam
Mat* camLib::getImg() {
    IplImage *newImg = cvQueryFrame(cap);   //Get new img, convert to Mat
    lastImg = new cv::Mat(cvarrToMat(newImg));
    return lastImg;
}


vector<pair<cv::Mat, cv::Point>> camLib::getBounds(Mat *img, bool display) { return getBounds(*img, display); }

//Returns array of Mats containing possible letters 
vector<pair<cv::Mat, cv::Point>> camLib::getBounds(Mat img, bool display) {
    //Main configuration values
    Scalar WHITE_MIN(0, 0, 90); //initial white filter min/max HSV
    Scalar WHITE_MAX(180, 65, 255);
    int CONTOUR_SIZE_MIN = 200;   //Remove contours smaller than this
    int LETTER_MIN_WIDTH = 40;
    int LETTER_MIN_HEIGHT = 40;
    float RATIO_THRESHOLD = .4;  //Removes contours with too high width:height ratio (value 0-1)

    Mat hsv;    //Conversion to HSV colour space
    cvtColor(img, hsv, CV_BGR2HSV);
    GaussianBlur(hsv, hsv, Size(15, 15), 0, 0);  //Smooth image with blur

    inRange(hsv, WHITE_MIN, WHITE_MAX, hsv);  //Filter out all colours except white/light grey
    if (display) imshow("Filtered HSV", hsv);

    std::vector<Mat> channels;
    split(hsv, channels);   //Splitting HSV space into 3 channels, using Hue for recognition

    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);

    Canny(Hue, Hue, 180, 0);    //Get lines, thicken x4 for better detection    
    for (int i = 0; i < 4; ++i)
        dilate(Hue, Hue, Mat());

    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  //Retrieving image contours, used to determine ROI's
    findContours(Hue, contoursH, hierarchyH, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    vector<pair<cv::Mat, cv::Point>> output;
    // draw the contours to a copy of the input image:
    for (int i = 0; i < contoursH.size(); i++) {
        if (contourArea(contoursH[i]) < CONTOUR_SIZE_MIN) continue; //Filter small contours
        if (hierarchyH[i][3] < 0) continue;  //Hollow out thick contours

        drawContours(img, contoursH, i, Scalar(0, 255, 0), 2, 5, hierarchyH, 0);


        cv::Rect boundRect = boundingRect(contoursH[i]);
        float ratio = (float) boundRect.width / (float) boundRect.height;
        if (ratio < 1.0 - RATIO_THRESHOLD || ratio > 1.0 + RATIO_THRESHOLD) continue;    //Filter rects too wide or tall

        drawContours(img, contoursH, i, Scalar(0, 255, 255), 2, 5, hierarchyH, 0);

        Mat cropped = origHue(boundRect); //Crop to rect


        //Retrieve contour closest to centre, crop to it
        std::vector<std::vector<cv::Point>> contoursOut;
        std::vector<cv::Vec4i> hierarchyOut;
        findContours(cropped, contoursOut, hierarchyOut, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
        if (contoursOut.size() <= 0) continue;
        vector<Rect> croppedRects;
        for (int y = 0; y < contoursOut.size(); y++) croppedRects.push_back(boundingRect(contoursOut.at(y)));
        double min = 10000;
        Rect target = croppedRects.at(0);
        Point centre(boundRect.width / 2, boundRect.height / 2);
        for (int y = 0; y < croppedRects.size(); y++) {
            double dist = cv::norm(Point(croppedRects.at(y).x + croppedRects.at(y).width / 2,
                                         croppedRects.at(y).y + croppedRects.at(y).height / 2) - centre);
            if (dist < 0) dist = dist * -1;
            if (dist < min) {
                min = dist;
                target = croppedRects.at(y);
            }
        }

        cropped = cropped(target);


        if (cropped.cols < LETTER_MIN_WIDTH || cropped.rows < LETTER_MIN_HEIGHT)
            continue;

        //Add cropped img to output vector, draw rects onto img
        centre = Point(boundRect.x + boundRect.width / 2, boundRect.y + boundRect.height / 2);
        output.push_back(make_pair(cropped, centre));;
        drawContours(img, contoursH, i, Scalar(0, 0, 255), 2, 8, hierarchyH, 0);
    }

    //Show input/output images
    if (display) {
        imshow("Hue", Hue);
        imshow("InverseHue", Scalar::all(255) - origHue);
        waitKey(10);
    }

    //Calculate FPS
    double fps = (std::clock() - timer) / (double) CLOCKS_PER_SEC * 100;
    fps = floor(60 / fps);
    timer = std::clock();
    //printf("FPS: %.0f\n", fps);

    return output;
}


//Runs OCRTesseract on input image
vector<Letter> camLib::findLetter(Mat img, bool display, int min_confidence) {
    vector<pair<cv::Mat, cv::Point>> regions = getBounds(img,
                                                         display);  //Retrieve vector of Mats possibly containing letters
    vector<Letter> let;
    Mat scan;
    for (int i = 0; i < regions.size(); i++) //Iterate through all retrieved Mats
    {
        if (regions[i].first.cols > 0) {
            std::string res = "";
            regions[i].first.copyTo(scan);    //Apply white border area around Mat
            copyMakeBorder(scan, scan, 50, 50, 50, 50, BORDER_CONSTANT, Scalar(0, 0, 0));
            scan = Scalar::all(255) - scan;     //Inverse colours to make letter black

            vector<Rect> *rects = new vector<Rect>();
            vector<std::string> *texts = new vector<std::string>();
            vector<float> *confidence = new vector<float>();
            tess->run(scan, res, rects, texts, confidence); //Runs Tesseract OCR on Mat

            //Ensure letter was found and filter low confidence
            if (res.length() == 3) { res = res.substr(0, 1); }
            if (!res.empty() && confidence->at(0) >= min_confidence) {
                let.push_back(Letter());    //Add letter to vector of type Letter
                int pos = ((float) regions[i].second.x / (float) img.cols) * 100;
                let.at(let.size() - 1).letter = char(res.at(0));
                let.at(let.size() - 1).pos = pos;
                let.at(let.size() - 1).width = regions[i].first.cols;
                let.at(let.size() - 1).height = regions[i].first.rows;
                if (display) {
                    imshow(("R" + to_string(i)), scan);
                    waitKey(5);
                    rectangle(img, Rect(regions[i].second.x - regions[i].first.cols / 2,
                                        regions[i].second.y - regions[i].first.rows / 2, regions[i].first.cols,
                                        regions[i].first.rows), Scalar(255, 0, 0), 4);
                }
            }
            delete rects;   //Free up memory pointers
            delete texts;
            delete confidence;
        }
    }
    if (display) { imshow("Output", img); }
    return let;
}


//Python Bindings
extern "C" {

}