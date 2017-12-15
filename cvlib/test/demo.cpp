#include <iostream>
#include "library.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv ) {
<<<<<<< HEAD

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

/*======
    camLib* camAPI = new camLib();
    while(true)
    {
            std::vector<Letter> lettrs = camAPI->findLetter(camAPI->getImg(), true);
            for(int i=0; i<lettrs.size(); i++)
                printf("Found: %c at %i%% to the left. Size: %i,%i  Ratio: %.2f\n", lettrs.at(i).letter, lettrs.at(i).pos, lettrs.at(i).width, lettrs.at(i).height, ((float)lettrs.at(i).width/(float)lettrs.at(i).height));
            if(lettrs.size()>0) std::cout << "---" << std::endl;
            //camAPI->getBounds(camAPI->getImg());
    }
>>>>>>> ColourOCR*/
}
