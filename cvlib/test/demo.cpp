#include <iostream>
#include "library.h"

int main(int argc, char** argv ) {
    camLib* camAPI = new camLib();
    while(true)
    {
            camAPI->findLetter(camAPI->getImg());
            //camAPI->getBounds(camAPI->getImg());
    }
}
