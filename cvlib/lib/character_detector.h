//
// Created by David Helekal on 04/11/2017.
//

#ifndef CVLIB_CHARACTER_DETECTOR_H
#define CVLIB_CHARACTER_DETECTOR_H


#include <vector>
#include <opencv2/core/types.hpp>
#include <opencv2/text.hpp>
#include <FTPyramid.hpp>
#include <Segmenter.h>

class character_detector {

public:
    character_detector();

    ~character_detector();

    std::vector<cv::Rect> character_bounds(cv::Mat &im);

private:
    void preprocess_image(cv::Mat &im, std::vector<cv::Mat> &channels);

    int nfeatures = 500;
    float scaleFactor = 1.2f;
    int nlevels = 8;
    int edgeThreshold = 50;
    int keypointTypes = 3;
    int kMin = 9;
    int kMax = 11;
    bool color = false;

    cv::Ptr<cmp::FTPyr> ftDetector;
    cv::Ptr<cmp::CharClassifier> charClassifier;
    cv::Ptr<cmp::Segmenter> segmenter;
};


#endif //CVLIB_CHARACTER_DETECTOR_H
