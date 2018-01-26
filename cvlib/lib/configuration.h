#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iostream>
#include <string>
#include <unistd.h>

class configuration
{
private:
    void readFile();
    void writeFile();
    std::map<string,string> store;
public:
    configuration();
    ~configuration();
    String getValue(string key);
    String updateValue(string val);
};

#endif // CONFIGURATION_H
