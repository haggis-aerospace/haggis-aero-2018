#ifndef CAM_CALIB
#define CAM_CALIB

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners);

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& foundCorners, bool showResult = false);

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients);

bool saveCameraCalibration(String file, Mat cameraMatrix, Mat distanceCoefficients);

#endif
