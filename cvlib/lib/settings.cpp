#include "settings.h"

using namespace std;

bool fileExists(const std::string& filename)
{
    struct stat buf;
    return (stat(filename.c_str(), &buf) != -1);
}

const vector<string> split(const string& s, const char& c)
{
    string buff{""};
    vector<string> v;
    for(auto n:s)
    {
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if(buff != "") v.push_back(buff);
    return v;
}


colourData::colourData(){}

colourData::~colourData(){}

int toInt(string s){return atoi(s.c_str());}

void colourData::readData()
{
    if(!fileExists("settings.cfg"))
    { cout << "WARNING: No config file found, use configurator to generate file" << endl; return; }
    
    string line;
    ifstream file("settings.cfg");
    getline(file, line);
    vector<string> values = split(line,',');
    W_MIN_H = toInt(values.at(0));
    W_MIN_S = toInt(values.at(1));
    W_MIN_V = toInt(values.at(2));
    
    W_MAX_H = toInt(values.at(3));
    W_MAX_S = toInt(values.at(4));
    W_MAX_V = toInt(values.at(5));
    
    RL_MIN_H = toInt(values.at(6));
    RD_MIN_H = toInt(values.at(7));
    R_MIN_S = toInt(values.at(8));
    R_MIN_V = toInt(values.at(9));
    
    RL_MAX_H = toInt(values.at(10));
    RD_MAX_H = toInt(values.at(11));
    R_MAX_S = toInt(values.at(12));
    R_MAX_V = toInt(values.at(13));
    
    cout << "Loaded colour data from file" << endl;
}

