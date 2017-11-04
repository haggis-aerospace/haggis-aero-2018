//
// Created by David Helekal on 04/11/2017.
//

#include <opencv/cv.hpp>
#include "character_detector.h"
#include "configuration.h"

character_detector::character_detector(float min_confidence, float min_area, float max_area) {
    this->min_area = min_area;
    this->max_area = max_area;
    this->min_confidence = min_confidence;

    filter1 = cv::text::createERFilterNM1(cv::text::loadClassifierNM1(std::string(RESOURCES_PATH)+"trained_classifierNM1.xml"), 1,
                                          min_area, max_area, min_confidence, true, min_confidence);
    filter2 = cv::text::createERFilterNM2(cv::text::loadClassifierNM2(std::string(RESOURCES_PATH)+"trained_classifierNM2.xml"), 0.4);
}

character_detector::~character_detector() {

}

std::vector<cv::Rect>
character_detector::character_bounds(cv::Mat &im) {

    std::vector<std::vector<cv::text::ERStat>> erstats;

    std::vector<cv::Mat> channels;
    preprocess_image(im, channels);

    for (int i = 0; i < 3; ++i) {
        erstats.push_back(std::vector<cv::text::ERStat>());
    }

    for (int i = 0; i < 3; ++i) {
        filter1->run(channels[i], erstats[i]);
        filter2->run(channels[i], erstats[i]);
    }

    std::vector<cv::Rect> out;

    for(auto c : erstats){
        for(auto er : c) {
            out.push_back(er.rect);
        }
    }

    return out;
}

void character_detector::preprocess_image(cv::Mat &im, std::vector<cv::Mat> &channels) {
    cv::resize(im, im, cv::Size(640, 480));
    cv::split(im, channels);
}
