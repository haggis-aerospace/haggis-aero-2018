#include <iostream>
#include <character_detector.h>
#include "library.h"

int main(int argc, char** argv ) {
    auto camAPI = new camLib();
    auto det = new character_detector();
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    while(true)
    {
        cv::Ptr<cv::Mat> im = camAPI->getImg();
                               // Wait for a keystroke in the window

        std::vector<cv::Rect> bboxes = det->character_bounds(*im);
        for(int i=0; i<bboxes.size(); i++) {
                cv::rectangle(*im, bboxes[i], cv::Scalar(255, 0, 0));
        }
        cv::imshow("Display window", *im);
        waitKey(25);
    }
}
