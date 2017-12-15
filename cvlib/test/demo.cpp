//
// Created by David Helekal on 14/10/2017.
//

#include <iostream>
#include "library.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv ) {

	Ptr<Stereo> test = new Stereo();
	test->loadIntrinsics();
	test->loadExtrinsics();

	while(true){
		test->getStereoImages();
		test->rectifyImages(true);
		//test->readImages();
		test->getDisparity();
		char c = waitKey(50);
	}

}
