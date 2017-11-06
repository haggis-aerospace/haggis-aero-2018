//
// Created by David Helekal on 04/11/2017.
//

#include <opencv/cv.hpp>
#include <character_detector.h>
#include <vis/componentsVis.h>
#include <configuration.h>

character_detector::character_detector() : ftDetector(cv::Ptr<cmp::FTPyr> (new cmp::FTPyr(nfeatures, scaleFactor, nlevels, edgeThreshold, keypointTypes, kMin, kMax, color, false, false))),
                                           charClassifier(cv::Ptr<cmp::CharClassifier> (new cmp::CvBoostCharClassifier(string(string(RESOURCES_PATH)+"cvBoostChar.xml").data()))),
                                           segmenter(cv::Ptr<cmp::Segmenter> (new cmp::PyramidSegmenter(ftDetector, charClassifier)))

{}

character_detector::~character_detector() {

}

std::vector<cv::Rect>
character_detector::character_bounds(cv::Mat &im) {

    cv::Mat gray;
    std::vector<cv::Rect> out;
    cv::cvtColor(im, gray, cv::COLOR_BGR2GRAY);

    std::vector<cmp::FastKeyPoint> keypts;
    std::unordered_multimap<int, std::pair<int, int> > keypts_pixels;
    std::vector<cmp::LetterCandidate*> letters;

    cv::Mat mask1, mask2;

    ftDetector->detect(gray, keypts, keypts_pixels);
    segmenter->getLetterCandidates(gray, keypts, keypts_pixels, letters);

    cv::Mat debug = gray.clone();

    std::vector<cmp::LetterCandidate>& candidates = segmenter->getLetterCandidates();
    cv::cvtColor(debug,debug, cv::COLOR_GRAY2BGR);
    cv::Mat cser = cmp::createCSERImage(letters, keypts, keypts_pixels, debug);

    cv::namedWindow("Debug", CV_WINDOW_AUTOSIZE);
    cv::imshow("Debug", cser);
    cv::waitKey(25);

    for(auto l : candidates) {
        out.emplace_back(cv::Rect(l.bbox));
    }

    return out;
}

void character_detector::preprocess_image(cv::Mat &im, std::vector<cv::Mat> &channels) {
}
