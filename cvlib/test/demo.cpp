//
// Created by David Helekal on 14/10/2017.
//

#include <iostream>
#include "library.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv ) {
	std::vector<cv::Mat> images;
	images.push_back(cv::imread("left.jpg",CV_LOAD_IMAGE_GRAYSCALE));
	images.push_back(cv::imread("right.jpg", CV_LOAD_IMAGE_GRAYSCALE));

	Ptr<Stereo> test = new Stereo();
	test->loadIntrinsics();
	test->loadExtrinsics();
	while(true){
		test->getStereoImages();
		test->rectifyImages(true);
		test->getDisparity();
		char c = waitKey(50);
	}

}
