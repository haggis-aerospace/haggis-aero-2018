#include "cameraCalibration.h"

const float CALIB_SQUARE_SIZE = 0.019f;
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


int main(int argc, char** argv){
	vector<Mat> frame, drawToFrame;
	vector<Mat> cameraMatrix;
	vector<Mat> distanceCoefficients;

	vector<vector<Mat>> savedImages;
	vector<vector<vector<Point2f>>> markerCorners, rejectedCandidates;

	vector<VideoCapture> cam;

	for(int i = 0; i < CAM_INDX.size(); i++){
		cameraMatrix[i] =  Mat::eye(3,3,CV_64F);
		if (!cam[i].read(frame)){
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
				imshow("webcam " + i,drawToFrame[i]);
			}
			else{
				imshow("webcam " + 1,frame[i]);
			}
		}
		char charachter = waitKey(50);

		switch (charachter) {
			case 32:
				//saving image
				for(int i = 0; i < CAM_INDX.size(); i++){
					if (found[i]){
						Mat temp;
						frame[i].copyTo(temp);
						savedImages[i].push_back(temp);
					} else {
						cout << "grid not detected in camera " << i << endl;
					}
				}
				break;
			case 10:
				//calibration
				for(int i = 0; i < CAM_INDX.size(); i++){
					if (savedImages[i].size() > 20){
						cameraCalibration(savedImages[i], BOARD_DIMENSIONS, CALIB_SQUARE_SIZE, cameraMatrix[i], distanceCoefficients[i]);
						saveCameraCalibration("Calibration"+i, cameraMatrix[i], distanceCoefficients[i]);
					} else {
						cout << "not enough images to calibrate camera " << i << endl;
					}
				}
				break;
			case 27:
				return 1;
				break;
		}
	}
	return 1;

}
