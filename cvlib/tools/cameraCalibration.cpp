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

	double rms = stereoCalibrate(objectPoints, points[0], points[1],
                    cameraMatrix[0], distanceCoefficients[0],
                    cameraMatrix[1], distanceCoefficients[1],
                    imgSize, R, T, E, F,CV_CALIB_FIX_INTRINSIC,
					TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 100, 1e-5) );

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

	//TODO delete this bullshit test stuff

	//-- 1. Read the images


	cv::cvtColor(calibrationImages[0], calibrationImages[0], CV_BGR2GRAY);

	cv::cvtColor(calibrationImages[1], calibrationImages[1], CV_BGR2GRAY);

	vector<Mat> rview(2), map1(2), map2(2);

	for (int i = 0; i < 2; i++){
		imshow("test" + to_string(i), calibrationImages[i]);
		initUndistortRectifyMap(
			cameraMatrix[i], distanceCoefficients[i], R,
			getOptimalNewCameraMatrix(cameraMatrix[i], distanceCoefficients[i], imgSize, 1, imgSize, 0), imgSize,
			CV_16SC2, map1[i], map2[i]);
		remap(calibrationImages[i], rview[i], map1[i], map2[i], INTER_LINEAR);
		rectangle(rview[i], validRoi[i], 50);
		imshow("qwe" + to_string(i), rview[i]);
	}



	/*
	//-- And create the image in which we will save our disparities
	Mat imgDisparity16S = Mat( calibrationImages[0].rows, calibrationImages[0].cols, CV_16S );
	Mat imgDisparity8U = Mat( calibrationImages[0].rows, calibrationImages[0].cols, CV_8UC1 );


	//-- 2. Call the constructor for StereoBM
	int ndisparities = 7*16;   //< Range of disparity
	int SADWindowSize = 9; //< Size of the block window. Must be odd

	Ptr<StereoBM> sbm = StereoBM::create( ndisparities, SADWindowSize );
	sbm->setROI1(validRoi[0]);
	sbm->setROI2(validRoi[1]);
	sbm->setPreFilterSize(5);
	sbm->setPreFilterCap(61);
	sbm->setMinDisparity(-39);
	sbm->setTextureThreshold(507);
	sbm->setUniquenessRatio(0);
	sbm->setSpeckleWindowSize(0);
	sbm->setSpeckleRange(8);
	sbm->setDisp12MaxDiff(1);
	//-- 3. Calculate the disparity image
	sbm->compute( calibrationImages[0], calibrationImages[1], imgDisparity16S );

	//-- Check its extreme values
	double minVal; double maxVal;

	minMaxLoc( imgDisparity16S, &minVal, &maxVal );

	printf("Min disp: %f Max value: %f \n", minVal, maxVal);

	//-- 4. Display it as a CV_8UC1 image
	imgDisparity16S.convertTo( imgDisparity8U, CV_8UC1, 255/(maxVal - minVal));

	namedWindow( "windowDisparity", WINDOW_NORMAL );
	imshow( "windowDisparity", imgDisparity8U );
	*/
	return rms;
}


int main(int argc, char** argv){
	vector<Mat> frame(CAM_INDX.size()), drawToFrame(CAM_INDX.size());
	vector<Mat> cameraMatrix(CAM_INDX.size());
	vector<Mat> distanceCoefficients(CAM_INDX.size());

	vector<Mat> savedImages;
	vector<vector<Mat>> savedImagesIn(CAM_INDX.size());
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
			undistort(frame[i],undist[i],cameraMatrix[i],distanceCoefficients[i]);
			imshow(("undist" + to_string(i)),undist[i]);
		}
		char charachter = waitKey(50);
		bool pairFound = true;
		Mat temp[CAM_INDX.size()],temp1[CAM_INDX.size()];
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
						frame[i].copyTo(temp[i]);
						frame[i].copyTo(temp1[i]);
						savedImages.push_back(temp[i]);
						savedImagesIn[i].push_back(temp1[i]);
					}
				}
				break;
			case 10:
				//calibration

					if (savedImages.size() > 10){ //calibrate extrinsics of the stereo cameras
						for(int i = 0; i < CAM_INDX.size(); i++){
							cameraCalibration(savedImagesIn[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
						}
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
				if (savedImages.size() > 10){ //calibrate camera intrinsics for each camera
					for(int i = 0; i < CAM_INDX.size(); i++){
						cameraCalibration(savedImagesIn[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
					}
				}
				break;
			case 27:
				return 1;
				break;
			case 67:
			case 99:
				savedImages.clear();// delete saved calibration images
		}
	}
	return 1;

}
