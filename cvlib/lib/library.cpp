#include "library.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;


//TODO add structure
//TODO add comments
//TODO make it work for 2 cams
//TODO complete header file
//TODO stereo vision
//TODO test!

void hello() {
    std::cout << "Hello, World!" << std::endl;
}
void libtest() {
     std::cout << cv::getBuildInformation();
}

void undistortDemo(){
	const vector<int> CAM_INDX = {1,2};

	vector<Mat> frame(2), fixedFrame(2);
	vector<Mat> cameraMatrix(CAM_INDX.size());
	vector<Mat> distanceCoefficients(CAM_INDX.size());

	vector<VideoCapture> cam(CAM_INDX.size());

	for(int i = 0; i < CAM_INDX.size(); i++){
		cameraMatrix[i] =  Mat::eye(3,3,CV_64F);
		vector<double> buff(14);
		ifstream fs("Calibration"+to_string(i));
		if(!fs){
			return;
		}

		for(int j = 0; j < 14; j++){
			fs >> buff[j];
		}

		fs.close();
		for(int j = 0; j < cameraMatrix[i].rows; j++){
			for(int k = 0; k < cameraMatrix[i].cols; k++){
				 cameraMatrix[i].at<double>(j,k) = buff[k + (j*cameraMatrix[i].cols)];
				 cout << cameraMatrix[i].at<double>(j,k) << ",";
			}
			cout << endl;
		}

		distanceCoefficients[i] = Mat::zeros(1, 5, CV_64F);
		for(int j = 0; j < 5; j++){
			distanceCoefficients[i].at<double>(j) = buff[9+j];

		}

		cam[i].open(CAM_INDX[i]);
		if (!cam[i].read(frame[i])){
			return;
		}

	}


	while(true){
		for (int i = 0; i < CAM_INDX.size(); i++){
			cam[i].read(frame[i]);
			frame[i].copyTo(fixedFrame[i]);
			undistort(frame[i],fixedFrame[i],cameraMatrix[i],distanceCoefficients[i]);
			imshow("Orig"+to_string(i),frame[i]);
			imshow("Fixed"+to_string(i),fixedFrame[i]);
			char temp = waitKey(20);
		}
	}
}
