#include <iostream>
#include "library.h"
#include <stddef.h>
#include <vector>
#include <thread>
#include <unistd.h>
using namespace std;


int main(int argc, char** argv ) {
    /*
    std::deque<double> frames;
    camLib* camAPI = new camLib(true);
    std::clock_t timer = std::clock();   //For calculating FPS
    
    std::vector<Letter> lettrs;
    Letter ltr;
    while(true)
    {
        camAPI->findLetter(camAPI->getImg());
        ltr = camAPI->mostOccouring();
       
        double fps = ( std::clock() - timer ) / (double) CLOCKS_PER_SEC * 100;
        
        printf("Found: %c at %i%% to the left. Size: %i,%i  Angle Ratio: %.2f\n", ltr.letter, ltr.pos, ltr.width, ltr.height, ((float)ltr.width/(float)ltr.height));
        
        //Calculate FPs
        frames.push_front(60.0/fps);
        if(frames.size() > 4)
            frames.pop_back();
        double totalfps = 0;
        for(int i=0; i<frames.size(); i++)
            totalfps = totalfps + frames[i];
        fps = totalfps/frames.size();
        printf("FPS: %.3f\n", fps);
        
        std::cout << "---" << std::endl;
        
        timer = std::clock();
    }
    */
    cout << "No current demo avaliable" << endl;
}
