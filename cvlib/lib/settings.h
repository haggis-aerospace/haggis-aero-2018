#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <vector>


class colourData
{
    public: 
        int W_MIN_H = 0;
        int W_MIN_S = 0;
        int W_MIN_V = 90;

        int W_MAX_H = 180;
        int W_MAX_S = 80;
        int W_MAX_V = 255;

        int RL_MIN_H = 160;
        int RD_MIN_H = 0;
        int R_MIN_S = 100;
        int R_MIN_V = 20;

        int RL_MAX_H = 180;
        int RD_MAX_H = 10;
        int R_MAX_S = 255;
        int R_MAX_V = 255;
        
        void saveData();
        void readData();
        colourData();
        ~colourData();
};

#endif // SETTINGS_H
