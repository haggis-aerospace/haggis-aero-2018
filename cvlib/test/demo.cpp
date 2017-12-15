#include <iostream>
#include "library.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv ) {
//<<<<<<< HEAD

	Ptr<Stereo> stereo = new Stereo();
	Ptr<camLib> ocr = new camLib();
	stereo->loadIntrinsics();
	stereo->loadExtrinsics();

	while(true){
		stereo->getStereoImages();
		stereo->rectifyImages(true);
		vector<Mat> img = stereo->getImages();

		//test->readImages();
		std::vector<Letter> lettrs = ocr->findLetter(img[0],true);
		stereo->getDisparity();
		for(int i=0; i<lettrs.size(); i++){
			printf("Found: %c at %i%% to the left. Size: %i,%i  Ratio: %.2f\n", lettrs.at(i).letter, lettrs.at(i).pos, lettrs.at(i).width, lettrs.at(i).height, ((float)lettrs.at(i).width/(float)lettrs.at(i).height));
			cout << stereo->distance(lettrs[i].rect) << "m" << endl;
		}

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
