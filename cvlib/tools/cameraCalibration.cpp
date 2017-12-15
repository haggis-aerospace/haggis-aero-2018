//#include "cameraCalibration.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

const float CALIB_SQUARE_SIZE = 0.035f;
const Size BOARD_DIMENSIONS = Size(6,9);
const vector<int> CAM_INDX = {1,2};


class Calib{
public:
	/*const float CALIB_SQUARE_SIZE = 0.0335f;
	const Size BOARD_DIMENSIONS = Size(6,9);
	const vector<int> CAM_INDX = {1,2};*/
	Mat R, T, E, F, R1, R2, P1, P2, Q, M1, M2, D1, D2;
	Rect validRoi[2];

	Calib();
	void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners);
	void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& foundCorners, bool showResult);
	void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat M, Mat D);
	double calibrateCameraStereo(vector<vector<Mat>> calibrationImages, Size boardSize, float squareEdgeLength);
	void loadIntrinsics();
	void saveIntrinsics();
	void loadExtrinsics();
	void saveExtrinsics();
};

Calib::Calib(){
	M1 = Mat::eye(3,3,CV_64F);
	M2 = Mat::eye(3,3,CV_64F);
	D1 = Mat::zeros(1,5,CV_64F);
	D2 = Mat::zeros(1,5,CV_64F);
}

void Calib::createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners){
	for (int i = 0; i<boardSize.height; i++){
		for (int j = 0; j < boardSize.width; j++){
			corners.push_back(Point3f(j*squareEdgeLength,i*squareEdgeLength,0.0f));
		}
	}
}

void Calib::getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& foundCorners, bool showResult){
	for (vector<Mat>::iterator i = images.begin(); i != images.end(); i++){
		vector<Point2f> points;
		bool found = findChessboardCorners(*i, BOARD_DIMENSIONS, points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE | CALIB_CB_FAST_CHECK);

		if (found){
			foundCorners.push_back(points);
		}

		if (showResult){
			drawChessboardCorners(*i, BOARD_DIMENSIONS, points, found);
			imshow("Corners", *i);
		}
	}
}

void Calib::cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat M, Mat D){
	vector<vector<Point2f>> chessboardImagePoints;
	getChessboardCorners(calibrationImages,chessboardImagePoints,false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize,squareEdgeLength,worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(chessboardImagePoints.size(),worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	//D = Mat::zeros(1, 5, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, chessboardImagePoints, boardSize, M, D, rVectors, tVectors);
}

double Calib::calibrateCameraStereo(vector<vector<Mat>> calibrationImages, Size boardSize, float squareEdgeLength){
	vector<vector<Point2f>> pointsTemp;
	vector<vector<vector<Point2f>>> points(2);
	Size imgSize = calibrationImages[0][0].size();
	vector<vector<Point3f>> objectPoints(1);

	getChessboardCorners(calibrationImages[0], points[0], false);
	getChessboardCorners(calibrationImages[1], points[1], false);
	cout << "test" << endl;

	createKnownBoardPosition(boardSize,squareEdgeLength,objectPoints[0]);
	objectPoints.resize(points[0].size(),objectPoints[0]);

	cout << "test" << endl;
	try{
	double rms = stereoCalibrate(objectPoints, points[0], points[1],
                    M1, D1, M2, D2,
                    imgSize, R, T, E, F,CV_CALIB_FIX_INTRINSIC,
					TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 100, 1e-5) );

	stereoRectify(M1, D1, M2, D2, imgSize, R, T, R1, R2, P1, P2, Q,
				  CALIB_ZERO_DISPARITY, 0, Size(0,0), &validRoi[0], &validRoi[1]);
	return rms;
	}
	catch(Exception e){

	}

	return -1.0;
}

void Calib::loadIntrinsics(){
	FileStorage fs("intrinsics.yml", FileStorage::READ);
	if( fs.isOpened() ){
		fs["M1"] >> M1;
		fs["M2"] >> M2;
		fs["D1"] >> D1;
		fs["D2"] >> D2;
		fs.release();
		cout << M1 << endl << endl << M2 << endl << endl;
		cout << D1 << endl << endl << D2 << endl << endl;
	}
}

void Calib::saveIntrinsics(){
	FileStorage fs("intrinsics.yml", FileStorage::WRITE);
	if( fs.isOpened() ){
		fs << "M1" << M1 << "D1" << D1 <<
			"M2" << M2 << "D2" << D2;
		fs.release();
	}
}

void Calib::saveExtrinsics(){
	FileStorage fs("extrinsics.yml", FileStorage::WRITE);
	if( fs.isOpened() )
	{
		fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 <<
		"P1" << P1 << "P2" << P2 << "Q" << Q;
		fs.release();
	}
}

int main(int argc, char** argv){
	vector<Mat> frame(CAM_INDX.size()), drawToFrame(CAM_INDX.size());

//	vector<Mat> savedImages;
	vector<vector<Mat>> savedImages(CAM_INDX.size());
	vector<vector<vector<Point2f>>> markerCorners(CAM_INDX.size()), rejectedCandidates(CAM_INDX.size());

	vector<VideoCapture> cam(CAM_INDX.size());

	for(int i = 0; i < CAM_INDX.size(); i++){
		cam[i].open(CAM_INDX[i]);
		if (!cam[i].read(frame[i])){
			return 0;
		}
	}

	Calib cal;

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
		}

		char charachter = waitKey(50);
		bool pairFound = true;
		Mat temp[CAM_INDX.size()];

		switch (charachter) {
			// Spc: save image pair
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
			// Entr: calibrate stereo extrinsics
			case 10:
					if (savedImages[0].size() > 10){
						cout << "calib error of " << to_string(cal.calibrateCameraStereo(savedImages, BOARD_DIMENSIONS, CALIB_SQUARE_SIZE)) << endl;
					} else {
						cout << "not enough images to calibrate cameras " << endl;
					}
			//	}
				break;
			// A or a: calibrate intrinsics for both cameras
			case 65:
			case 97:
				if (savedImages[0].size() > 10){ //calibrate camera intrinsics for each camera
						cal.cameraCalibration(savedImages[0], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cal.M1, cal.D1);
						cal.cameraCalibration(savedImages[1], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cal.M2, cal.D2);
						savedImages[0].clear();
						savedImages[1].clear();
						cout << cal.M1 << endl << cal.D1 << endl;
						cout << cal.M2 << endl << cal.D2 << endl;
						cout << "intrinsics calibrated" << endl;
				}

				break;
			// esc: close program
			case 27:
				return 1;
				break;
			// C/c: clear all images
			case 67:
			case 99:
				savedImages[0].clear();
				savedImages[1].clear();
				cout<<"images cleared" <<endl;
				break;
			// l: save image for left camera
			case 108:
				if (!found[0]){
					cout << "grid not detected in camera " << 0 << endl;
				} else {
					frame[0].copyTo(temp[0]);
					savedImages[0].push_back(temp[0]);
				}
				break;
			// r: save image for right camera
			case 114:
				if (!found[1]){
					cout << "grid not detected in camera " << 1 << endl;
				} else {
					frame[1].copyTo(temp[1]);
					savedImages[1].push_back(temp[1]);
				}
				break;
			// L: calibrate lef camera intrinsics
			case 76:
				if (savedImages[0].size() > 10){ //calibrate camera intrinsics for each camera
					cal.cameraCalibration(savedImages[0], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cal.M1, cal.D1);
					cout << "left calibrated intrinsics" << endl;
					cout << cal.M1 << endl << cal.D1 << endl;
					savedImages[0].clear();
					savedImages[1].clear();
				} else {
					cout << "not enough images" << endl;
				}
				break;
			// R: calibrate right camera intrinsics
			case 82:
				if (savedImages[1].size() > 10){
					cal.cameraCalibration(savedImages[1], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cal.M2, cal.D2);
					cout << "right calibrated intrinsics" << endl;
					cout << cal.M2 << endl << cal.D2 << endl;
					savedImages[0].clear();
					savedImages[1].clear();
				} else {
					cout << "no enough images" << endl;
				}
				break;
			// i: load intrinsics
			case 105:
				cal.loadIntrinsics();
				cout << "intrinsics loaded" << endl;
				break;
			// I: save intrinsics
			case 73:
				cal.saveIntrinsics();
				cout<<"intrinsics saved" << endl;
				break;
			// E: save extrinsics
			 case 69:
				cal.saveExtrinsics();
				cout<<"extrinsics saved" << endl;
				break;
		}
	}
	return 1;

}
