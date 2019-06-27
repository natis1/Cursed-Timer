#include <iostream>
#include <vector>
#include <stdlib.h>
#include "display.h"


void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out)
{
    size_t start;
    size_t end = 0;
    
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

double parseTimerLength(std::string rawLength) {
    std::vector<std::string> stringList;
    tokenize(rawLength, ':', stringList);
    
    int multiplier = 1;
    double timeNeeded = 0;
    for (int i = stringList.size() - 1; i >= 0; i--) {
        if (multiplier == 1) {
            timeNeeded += atof(stringList.at(i).c_str()) * multiplier; 
        } else {
            timeNeeded += atoi(stringList.at(i).c_str()) * multiplier;
        }
        
        if (multiplier <= 3600) {
            multiplier *= 60;
        } else {
            multiplier *= 24;
        }
    }
    
    return timeNeeded;
    
    
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: cursedtimer \"Timer name\" \"Timer length\"" << std::endl;
        return 0;
    }
    std::string timerName = argv[1];
    std::string unparsedTimerLength = argv[2];
    double length = parseTimerLength(unparsedTimerLength);
    display d;
    d.timeTotal = length;
    d.timerName = timerName;
    d.startDisplay();
    
}
