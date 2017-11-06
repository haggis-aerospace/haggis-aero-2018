#include "library.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

const float CALIB_SQUARE_SIZE = 0.019f;
const Size BOARD_DIMENSIONS = Size(6,9);

void hello() {
    std::cout << "Hello, World!" << std::endl;
}
void libtest() {
     std::cout << cv::getBuildInformation();
}

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners){
	for (int i = 0; i<boardSize.height; i++){
		for (int j = 0; j < boardSize.width; j++){
			corners.push_back(Point3f(j*squareEdgeLength,i*squareEdgeLength,0.0f));
		}
	}
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& foundCorners, bool showResult = false){
	for (vector<Mat>::iterator i = images.begin(); i != images.end(); i++){
		vector<Point2f> points;
		bool found = findChessboardCorners(*i, BOARD_DIMENSIONS, points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found){
			foundCorners.push_back(points);
		}

		if (showResult){
			drawChessboardCorners(*i, BOARD_DIMENSIONS, points, found);
			imshow("Corners", *i);
			waitKey(0);
		}
	}
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients){
	vector<vector<Point2f>> chessboardImagePoints;
	getChessboardCorners(calibrationImages,chessboardImagePoints,false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize,squareEdgeLength,worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(chessboardImagePoints.size(),worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, chessboardImagePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

void runCalibration(bool showResult){
	Mat frame, drawToFrame;
	Mat cameraMatrix = Mat::eye(3,3,CV_64F);
	Mat distanceCoefficients;

	vector<Mat> savedImages;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture cam(1);

	if (!cam.read(frame)){
		return;
	}

	while (true){
		if(!cam.read(frame)){
			break;
		}

		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, BOARD_DIMENSIONS, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame,BOARD_DIMENSIONS,foundPoints,found);
		if (found && showResult){
			imshow("webcam",drawToFrame);
		}
		else{
			imshow("webcam",frame);
		}

		char charachter = waitKey(50);
		cout << (int)charachter << endl;

		switch (charachter) {
			case 32:
				//saving image
				if (found){
					Mat temp;
					frame.copyTo(temp);
					savedImages.push_back(temp);
				}
				break;
			case 10:
				//calibration
				if (savedImages.size() > 20){
					cameraCalibration(savedImages, BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix, distanceCoefficients);
					cout << "shit" << endl;
				} else {
					cout << "not enough images" << endl;
				}
				break;
		}
	}


}


void camTest() {
	VideoCapture c0(0),c1(1),c2(2);
	Mat f0,f1,f2;
	while(true){
		c0.read(f0);
		c1.read(f1);
		c2.read(f2);
		imshow("0",f0);
		imshow("1",f1);
		imshow("2",f2);
		waitKey(30);
	}
}
