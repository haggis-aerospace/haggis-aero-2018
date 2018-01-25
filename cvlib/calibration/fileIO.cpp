#include "fileIO.h"
#include <string>
#include <sys/stat.h>
#include <vector>

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


colourData::colourData()
{
    if(!fileExists("settings.cfg"))
        saveData();
        
    readData();
}

colourData::~colourData()
{
}

void colourData::saveData()
{
    ofstream outFile;
    outFile.open("settings.cfg");

    outFile << W_MIN_H << ",";
    outFile << W_MIN_S << ",";
    outFile << W_MIN_V << ",";

    outFile << W_MAX_H << ",";
    outFile << W_MAX_S << ",";
    outFile << W_MAX_V << ",";

    outFile << RL_MIN_H << ",";
    outFile << RD_MIN_H << ",";
    outFile << R_MIN_S  << ",";
    outFile << R_MIN_V  << ",";

    outFile << RL_MAX_H << ",";
    outFile << RD_MAX_H << ",";
    outFile << R_MAX_S  << ",";
    outFile << R_MAX_V;
    
    outFile.close();
}

int toInt(string s){return atoi(s.c_str());}

void colourData::readData()
{
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
}

