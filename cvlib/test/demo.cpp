#include <iostream>
#include "library.h"

int main(int argc, char** argv ) {
    camLib* camAPI = new camLib();
    while(true)
    {
            std::vector<Letter> lettrs = camAPI->findLetter(camAPI->getImg(), true);
            for(int i=0; i<lettrs.size(); i++)
                printf("Found: %c at %i%% to the left. Size: %i,%i  Ratio: %.2f\n", lettrs.at(i).letter, lettrs.at(i).pos, lettrs.at(i).width, lettrs.at(i).height, ((float)lettrs.at(i).width/(float)lettrs.at(i).height));
            if(lettrs.size()>0) std::cout << "---" << std::endl;
            //camAPI->getBounds(camAPI->getImg());
    }
}
