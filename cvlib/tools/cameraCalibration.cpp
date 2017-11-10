//#include "cameraCalibration.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

const float CALIB_SQUARE_SIZE = 0.0292f;
const Size BOARD_DIMENSIONS = Size(6,7);
const vector<int> CAM_INDX = {1,2};

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners){
	for (int i = 0; i<boardSize.height; i++){
		for (int j = 0; j < boardSize.width; j++){
			corners.push_back(Point3f(j*squareEdgeLength,i*squareEdgeLength,0.0f));
		}
	}
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& foundCorners, bool showResult){
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

bool saveCameraCalibration(String file, Mat cameraMatrix, Mat distanceCoefficients){
	ofstream outStream(file);
	if (outStream){
		uint16_t rows = cameraMatrix.rows, columns = cameraMatrix.cols;

		for (int r = 0; r < rows; r++){
			for(int c = 0; c < columns; c++){
				double val = cameraMatrix.at<double>(r,c);
				outStream << val << endl;
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		for (int r = 0; r < rows; r++){
			for(int c = 0; c < columns; c++){
				double val = distanceCoefficients.at<double>(r,c);
				outStream << val << endl;
			}
		}
		outStream.close();
		return true;
	}
	return false;

}

double calibrateCameraStereo(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, vector<Mat>& cameraMatrix, vector<Mat>& distanceCoefficients){
	vector<vector<Point2f>> pointsTemp;
	vector<vector<vector<Point2f>>> points(2);
	Size imgSize = calibrationImages[0].size();
	Mat R, T, E, F;
	vector<vector<Point3f>> objectPoints(1);

	getChessboardCorners(calibrationImages, pointsTemp, false);

	for(int i = 0; i < pointsTemp.size(); i++){
		points[i%2].push_back(pointsTemp[i]);
	}
	//cout << pointsTemp[0] << endl;

	createKnownBoardPosition(boardSize,squareEdgeLength,objectPoints[0]);
	objectPoints.resize(pointsTemp.size()/2,objectPoints[0]);
	//cout << objectPoints[0] << endl;

	return stereoCalibrate(objectPoints, points[0], points[1],
                    cameraMatrix[0], distanceCoefficients[0],
                    cameraMatrix[1], distanceCoefficients[1],
                    imgSize, R, T, E, F,
                    CALIB_FIX_ASPECT_RATIO +
                    CALIB_ZERO_TANGENT_DIST +
                    CALIB_USE_INTRINSIC_GUESS +
                    CALIB_SAME_FOCAL_LENGTH +
                    CALIB_RATIONAL_MODEL +
                    CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5,
					TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 100, 1e-5) );

}


int main(int argc, char** argv){
	vector<Mat> frame(CAM_INDX.size()), drawToFrame(CAM_INDX.size());
	vector<Mat> cameraMatrix(CAM_INDX.size());
	vector<Mat> distanceCoefficients(CAM_INDX.size());

	vector<Mat> savedImages;
	vector<vector<vector<Point2f>>> markerCorners(CAM_INDX.size()), rejectedCandidates(CAM_INDX.size());

	vector<VideoCapture> cam(CAM_INDX.size());

	for(int i = 0; i < CAM_INDX.size(); i++){
		cameraMatrix[i] =  Mat::eye(3,3,CV_64F);
		cam[i].open(CAM_INDX[i]);
		if (!cam[i].read(frame[i])){
			return 0;
		}
	}

	while (true){
		for(int i = 0; i < CAM_INDX.size(); i++){
			if(!cam[i].read(frame[i])){
				break;
			}
		}
		vector<bool> found = {false,false};
		for(int i = 0; i < CAM_INDX.size(); i++){
			vector<Vec2f> foundPoints;


			found[i] = findChessboardCorners(frame[i], BOARD_DIMENSIONS, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
			frame[i].copyTo(drawToFrame[i]);
			drawChessboardCorners(drawToFrame[i],BOARD_DIMENSIONS,foundPoints,found[i]);
			if (found[i]){
				imshow(("webcam" + to_string(i)),drawToFrame[i]);
			}
			else{
				imshow(("webcam" + to_string(i)),frame[i]);
			}
		}
		char charachter = waitKey(50);
		bool pairFound = true;
		Mat temp[CAM_INDX.size()];
		switch (charachter) {
			case 32:
				//saving image
				for(int i = 0; i < CAM_INDX.size(); i++){
					if (!found[i]){
						pairFound = false;
						cout << "grid not detected in camera " << i << endl;
					} else {
						frame[i].copyTo(temp[i]);
					}
				}
				if (pairFound){
					for(int i = 0; i < CAM_INDX.size(); i++){
						frame[i].copyTo(temp[i]);
						savedImages.push_back(temp[i]);
					}
				}
				break;
			case 10:
				//calibration
			//	for(int i = 0; i < CAM_INDX.size(); i++){
					if (savedImages.size() > 20){
						//cameraCalibration(savedImages[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
						//saveCameraCalibration("Calibration"+to_string(i), cameraMatrix[i], distanceCoefficients[i]);
						cout << "calib error of " << to_string(calibrateCameraStereo(savedImages, BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix, distanceCoefficients)) << endl;
					} else {
						cout << "not enough images to calibrate cameras " << endl;
					}
			//	}
				break;
			case 27:
				return 1;
				break;
		}
	}
	return 1;

}
