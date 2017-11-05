//
// Created by David Helekal on 04/11/2017.
//

#include <opencv/cv.hpp>
#include <character_detector.h>
#include <vis/componentsVis.h>

character_detector::character_detector(float min_confidence, float min_area, float max_area) :
    min_area(min_area),
    max_area(max_area),
    min_confidence(min_confidence),
    ft(new cmp::FTPyr()),
    seg(new cmp::PyramidSegmenter(ft))
{}

character_detector::~character_detector() {

}

std::vector<cv::Rect>
character_detector::character_bounds(cv::Mat &im) {

    std::vector<cv::Rect> out;
    cv::Mat imc = im.clone();

    std::vector<cmp::FastKeyPoint> keypts;
    std::unordered_multimap<int, std::pair<int, int> > mm;
    std::vector<cv::Mat> channels;
    std::vector<cmp::LetterCandidate*> lc;

    preprocess_image(imc, channels);

    ft->detect(imc, keypts, mm);
    seg->getLetterCandidates(imc, keypts, mm, lc, cv::Mat(), 50);

    cv::Mat debug = imc.clone();

    std::vector<cmp::LetterCandidate>& candidates = seg->getLetterCandidates();

    cv::cvtColor(debug,debug, cv::COLOR_GRAY2BGR);
    cv::Mat cser = cmp::createCSERImage(lc, keypts, mm, debug);

    cv::namedWindow("Debug", CV_WINDOW_AUTOSIZE);
    cv::imshow("Debug", cser);
    cv::waitKey(25);

    for(auto l : candidates) {
        out.emplace_back(cv::Rect(l.bbox));
    }

    return out;
}

void character_detector::preprocess_image(cv::Mat &im, std::vector<cv::Mat> &channels) {
   cv::cvtColor(im, im, cv::COLOR_BGR2GRAY);
}
