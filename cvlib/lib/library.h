#include "camLib.h"

#ifndef CVLIB_LIBRARY_H
#define CVLIB_LIBRARY_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

void imgRecTest();

extern "C"
{
    void hello();
    void libtest();
	void undistortDemo();
}

class Stereo{
private:
	cv::Mat M1, M2, D1, D2, R, T, R1, R2, P1, P2, Q, mapL1, mapL2, mapR1, mapR2, disp;
	cv::Rect Roi1, Roi2;
	std::vector<cv::Mat> frame;
	const int camIndx[2] = {1,2};
	cv::Ptr<cv::VideoCapture> cam[2];
	double distMultiplier;
	int crop, textureThreshold, minDisparity, speckleRange, preFilterCap, speckleSize, uniqnessRatio, ndisparities, SADWindowSize, preFilterSize;
public:
	Stereo();

	void loadIntrinsics(std::string file = "intrinsics.yml");
	void loadExtrinsics(std::string file = "extrinsics.yml");

	void getStereoImages(bool display = false);

	void rectifyImages(bool display = false);

	void getDisparity(bool display = false);

	std::vector<cv::Mat> getImages();

	double distance(cv::Rect rect);

	void readImages();//debug code

};


#endif
