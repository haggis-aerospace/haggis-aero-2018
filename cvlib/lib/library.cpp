#include "library.h"

#include "opencv2/ximgproc/disparity_filter.hpp"

using namespace cv;
using namespace std;


//TODO add structure
//TODO add comments
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
		ifstream fs("Calibration"+/*to_string(i)*/to_string(0));//change this
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

Stereo::Stereo(){
	frame.resize(2);
	for(int i = 0; i < 2; i++){
		cam[i] = new VideoCapture(camIndx[i]);
	}
	getStereoImages(false);
	loadIntrinsics();
	loadExtrinsics();
	/*stereoRectify(M1, D1, M2, D2, frame[0].size(), R, T, R1, R2, P1, P2, Q, CV_CALIB_ZERO_DISPARITY, 1, Size(), &Roi1, &Roi2);
	initUndistortRectifyMap(M1, D1, R1, P1, frame[0].size(), CV_16SC2, mapL1, mapL2);
	initUndistortRectifyMap(M2, D2, R2, P2, frame[1].size(), CV_16SC2, mapR1, mapR2);*/

	cv::namedWindow("test",1);
	textureThreshold = 500;
	createTrackbar("textureThreshold", "test", &textureThreshold, 1000);
	minDisparity = 106;
	createTrackbar("minDisparity", "test", &minDisparity, 200);
	speckleRange = 14;
	createTrackbar("speckleRange", "test", &speckleRange, 50);
	preFilterCap = 62;
	createTrackbar("preFilterCap", "test", &preFilterCap, 62);
	speckleSize = 31;
	createTrackbar("speckleSize", "test", &speckleSize, 100);
	uniqnessRatio = 3;
	createTrackbar("uniqnessRatio", "test", &uniqnessRatio, 100);
	ndisparities = 8;
	createTrackbar("ndisparities", "test", &ndisparities, 100);
	SADWindowSize = 8;
	createTrackbar("SADWindowSize", "test", &SADWindowSize, 50);
	preFilterSize = 5;
	createTrackbar("preFileterSize", "test", &preFilterSize, 100);
	crop = 100;
	createTrackbar("crop", "test", &crop, 100);
}

void Stereo::loadIntrinsics(std::string file){
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

void Stereo::loadExtrinsics(std::string file){
	FileStorage fs("extrinsics.yml", FileStorage::READ);
	if( fs.isOpened() ){
		fs["R"] >> R;
		fs["T"] >> T;
		fs["R1"] >> R1;
		fs["R2"] >> R2;
		fs["P1"] >> P1;
		fs["P2"] >> P2;
		fs["Q"] >> Q;
		fs.release();
	}
}

void Stereo::getStereoImages(/*std::vector<cv::Mat>& images, */bool display){
	for(int i = 0; i < 2; i++){
		cam[i]->read(frame[i]);
	}
	if(display){
		imshow("im0",frame[0]);
		imshow("im1",frame[1]);
	}
}


void Stereo::rectifyImages(/*std::vector<cv::Mat>& imagesIn, std::vector<cv::Mat>& imagesOut, */bool display){
	stereoRectify(M1, D1, M2, D2, frame[0].size(), R, T, R1, R2, P1, P2, Q, CV_CALIB_ZERO_DISPARITY, float(crop)/100,frame[0].size(), &Roi1, &Roi2);
	initUndistortRectifyMap(M1, D1, R1, P1, frame[0].size(), CV_16SC2, mapL1, mapL2);
	initUndistortRectifyMap(M2, D2, R2, P2, frame[1].size(), CV_16SC2, mapR1, mapR2);
	Mat tempL, tempR;

	distMultiplier = Q.at<double>(2,3)*(Q.at<double>(3,2));
	/*frame[0].copyTo(tempL);
	frame[1].copyTo(tempR);*/
	remap(frame[0], tempL, mapL1, mapL2, INTER_LINEAR);
	remap(frame[1], tempR, mapR1, mapR2, INTER_LINEAR);
	/*resize(frame[0],frame[0],Size(frame[0].cols,frame[0].rows));
	resize(frame[1],frame[1],Size(frame[1].cols,frame[1].rows));*/
	rectangle(tempL,Roi1,Scalar(0, 255, 0));
	rectangle(tempR,Roi2,Scalar(0, 255, 0));
	tempL.copyTo(frame[0]);
	tempR.copyTo(frame[1]);
	if(display){
		Mat disp(tempL.rows,tempL.cols*2,CV_8U);
		hconcat(tempL, tempR, disp);
		for(int i = 0; i < disp.rows/50; i++){
			line(disp, Point(0,i*50), Point(disp.cols,i*50), Scalar(0, 0, 255));
		}
		imshow("rec0",disp);
		//imshow("rec1",frame[1]);
	}
}

void Stereo::getDisparity(/*vector<Mat>& images, */bool display){
	Mat imgDisparity16S = Mat( frame[0].rows, frame[0].cols, CV_16S );
	Mat imgDisparity8U = Mat( frame[0].rows, frame[0].cols, CV_8UC1 );

	//-- 2. Call the constructor for StereoBM
	/*int ndisparities = 1;   //< Range of disparity
	int SADWindowSize = 15; //< Size of the block window. Must be odd*/

	if(display){
		imshow("color0",frame[0]);
		imshow("color1",frame[1]);
	}

	cv::cvtColor(frame[0], frame[0], CV_BGR2GRAY);
	cv::cvtColor(frame[1], frame[1], CV_BGR2GRAY);

	if(display){
		imshow("0",frame[0]);
		imshow("1",frame[1]);
	}

	Ptr<StereoBM> sbm = StereoBM::create(ndisparities*16,(SADWindowSize*2)+5);
	sbm->setROI1(Roi1);
	sbm->setROI2(Roi2);
	sbm->setPreFilterSize((preFilterSize*2)+5);
	sbm->setPreFilterCap(preFilterCap+1);//61);
	sbm->setMinDisparity(minDisparity-100);//-39);
	sbm->setTextureThreshold(textureThreshold);//507);
	sbm->setUniquenessRatio(uniqnessRatio);
	sbm->setSpeckleWindowSize(speckleSize);
	sbm->setSpeckleRange(speckleRange);//8);
	sbm->setDisp12MaxDiff(1);
	//-- 3. Calculate the disparity image
	sbm->compute( frame[0], frame[1], imgDisparity16S );

	//-- Check its extreme values
	double minVal; double maxVal;

	minMaxLoc( imgDisparity16S, &minVal, &maxVal );

	printf("Min disp: %f Max value: %f \n", distMultiplier/minVal, distMultiplier/maxVal);

	//-- 4. Display it as a CV_8UC1 image
	imgDisparity16S.convertTo( imgDisparity8U, CV_8UC1, 255/(maxVal - minVal));

	imshow( "test", imgDisparity8U );
}

void Stereo::readImages(){
	frame[0] = imread("aloeL.jpg");
	frame[1] = imread("aloeR.jpg");
}
