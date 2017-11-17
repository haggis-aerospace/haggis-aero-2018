//#include "cameraCalibration.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

const float CALIB_SQUARE_SIZE = 0.0335f;
const Size BOARD_DIMENSIONS = Size(6,9);
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

double calibrateCameraStereo(vector<vector<Mat>> calibrationImages, Size boardSize, float squareEdgeLength, vector<Mat>& cameraMatrix, vector<Mat>& distanceCoefficients){
	vector<vector<Point2f>> pointsTemp;
	vector<vector<vector<Point2f>>> points(2);
	Size imgSize = calibrationImages[0][0].size();
	Mat R, T, E, F;
	vector<vector<Point3f>> objectPoints(1);

	getChessboardCorners(calibrationImages[0], points[0], false);
	getChessboardCorners(calibrationImages[1], points[1], false);
	cout << "test" << endl;

	createKnownBoardPosition(boardSize,squareEdgeLength,objectPoints[0]);
	objectPoints.resize(points[0].size(),objectPoints[0]);

	cout << "test" << endl;
	double rms = stereoCalibrate(objectPoints, points[0], points[1],
                    cameraMatrix[0], distanceCoefficients[0],
                    cameraMatrix[1], distanceCoefficients[1],
                    imgSize, R, T, E, F,CV_CALIB_FIX_INTRINSIC,
					TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 100, 1e-5) );

					cout << "test" << endl;


	FileStorage fs("intrinsics.yml", FileStorage::WRITE);
	if( fs.isOpened() ){
		fs << "M1" << cameraMatrix[0] << "D1" << distanceCoefficients[0] <<
			"M2" << cameraMatrix[1] << "D2" << distanceCoefficients[1];
		fs.release();
	}
	cout << cameraMatrix[0] << endl << endl << cameraMatrix[1] << endl << endl;
	cout << distanceCoefficients[0] << endl << endl << distanceCoefficients[1] << endl << endl;


	Mat R1, R2, P1, P2, Q;
	Rect validRoi[2];

	stereoRectify(cameraMatrix[0], distanceCoefficients[0],
				  cameraMatrix[1], distanceCoefficients[1],
				  imgSize, R, T, R1, R2, P1, P2, Q,
				  CALIB_ZERO_DISPARITY, 0, Size(0,0), &validRoi[0], &validRoi[1]);

	fs.open("extrinsics.yml", FileStorage::WRITE);
	if( fs.isOpened() )
	{
		fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
		fs.release();
	}

	cv::cvtColor(calibrationImages[0][0], calibrationImages[0][0], CV_BGR2GRAY);

	cv::cvtColor(calibrationImages[1][0], calibrationImages[1][0], CV_BGR2GRAY);

	vector<Mat> rview(2), map1(2), map2(2);

	vector<Mat> P(2);
	P1.copyTo(P[0]);
	P2.copyTo(P[1]);

	for (int i = 0; i < 2; i++){
		imshow("test" + to_string(i), calibrationImages[i][0]);
		initUndistortRectifyMap(
			cameraMatrix[i], distanceCoefficients[i], R,
			P[i], imgSize,
			CV_16SC2, map1[i], map2[i]);
		remap(calibrationImages[i][0], rview[i], map1[i], map2[i], INTER_LINEAR);
		rectangle(rview[i], validRoi[i], 50);
		imshow("qwe" + to_string(i), rview[i]);
	}

	return rms;
}


int main(int argc, char** argv){
	vector<Mat> frame(CAM_INDX.size()), drawToFrame(CAM_INDX.size());
	vector<Mat> cameraMatrix(CAM_INDX.size());
	vector<Mat> distanceCoefficients(CAM_INDX.size());

//	vector<Mat> savedImages;
	vector<vector<Mat>> savedImages(CAM_INDX.size());
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
		vector<Mat> undist(2);

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
		//	undistort(frame[i],undist[i],cameraMatrix[i],distanceCoefficients[i]);
		//	imshow(("undist" + to_string(i)),undist[i]);
		}

		char charachter = waitKey(50);
		bool pairFound = true;
		Mat temp[CAM_INDX.size()];

		switch (charachter) {
			case 32:
				for(int i = 0; i < CAM_INDX.size(); i++){ //save image from both webcams
					if (!found[i]){
						pairFound = false;
						cout << "grid not detected in camera " << i << endl;
					} else {
						frame[i].copyTo(temp[i]);
					}
				}
				if (pairFound){
					for(int i = 0; i < CAM_INDX.size(); i++){
						//savedImages.push_back(temp[i]);
						savedImages[i].push_back(temp[i]);
						cout << "PAIR FOUND" << endl;
					}
				}
				break;
			case 10:
				//calibration

					if (savedImages[0].size() > 10){ //calibrate extrinsics of the stereo cameras
						/*for(int i = 0; i < CAM_INDX.size(); i++){
							cameraCalibration(savedImages[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
						}*/
						cout << cameraMatrix[0] << endl << endl << cameraMatrix[1] << endl << endl;
						cout << distanceCoefficients[0] << endl << endl << distanceCoefficients[1] << endl << endl;
						//saveCameraCalibration("Calibration"+to_string(i), cameraMatrix[i], distanceCoefficients[i]);
						cout << "calib error of " << to_string(calibrateCameraStereo(savedImages, BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix, distanceCoefficients)) << endl;
					} else {
						cout << "not enough images to calibrate cameras " << endl;
					}
			//	}
				break;
			case 65:
			case 97:
				if (savedImages[0].size() > 10){ //calibrate camera intrinsics for each camera
					for(int i = 0; i < CAM_INDX.size(); i++){
						cameraCalibration(savedImages[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
					}
				}
				break;
			case 27:
				return 1;
				break;
			case 67:
			case 99:
				savedImages[0].clear();
				savedImages[1].clear();
				cameraMatrix[0] =  Mat::eye(3,3,CV_64F);
				cameraMatrix[1] =  Mat::eye(3,3,CV_64F);
				distanceCoefficients.clear();
				distanceCoefficients.resize(CAM_INDX.size());
			case 108:
				if (!found[0]){
					cout << "grid not detected in camera " << 0 << endl;
				} else {
					frame[0].copyTo(temp[0]);
					savedImages[0].push_back(temp[0]);
				}
				break;
			case 114:
				if (!found[1]){
					cout << "grid not detected in camera " << 1 << endl;
				} else {
					frame[1].copyTo(temp[1]);
					savedImages[1].push_back(temp[1]);
				}
				break;
			case 76:
				if (savedImages[0].size() > 10){ //calibrate camera intrinsics for each camera
					cameraCalibration(savedImages[0], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[0], distanceCoefficients[0]);
				}
				break;
			case 82:
				if (savedImages[1].size() > 10){ //calibrate camera intrinsics for each camera
					cameraCalibration(savedImages[1], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[1], distanceCoefficients[1]);
				}
				break;
		}
	}
	return 1;

}
