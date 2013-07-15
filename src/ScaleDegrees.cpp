#include "ScaleDegrees.h"
#include <string>

std::map<std::string, int> ScaleDegrees::scaleDegreesMap;

void ScaleDegrees::init()
{
    scaleDegreesMap.insert(std::make_pair("1", 0));
    scaleDegreesMap.insert(std::make_pair("b2", 1));
    scaleDegreesMap.insert(std::make_pair("m2", 1));
    scaleDegreesMap.insert(std::make_pair("2", 2));
    scaleDegreesMap.insert(std::make_pair("M2", 2));
    scaleDegreesMap.insert(std::make_pair("#2", 3));
    scaleDegreesMap.insert(std::make_pair("b3", 3));
    scaleDegreesMap.insert(std::make_pair("m3", 3));
    scaleDegreesMap.insert(std::make_pair("3", 4));
    scaleDegreesMap.insert(std::make_pair("M3", 4));
    scaleDegreesMap.insert(std::make_pair("#3", 5));
    scaleDegreesMap.insert(std::make_pair("b4", 4));
    scaleDegreesMap.insert(std::make_pair("4", 5));
    scaleDegreesMap.insert(std::make_pair("P4", 5));
    scaleDegreesMap.insert(std::make_pair("#4", 6));
    scaleDegreesMap.insert(std::make_pair("b5", 6));
    scaleDegreesMap.insert(std::make_pair("d5", 6));
    scaleDegreesMap.insert(std::make_pair("5", 7));
    scaleDegreesMap.insert(std::make_pair("P5", 7));
    scaleDegreesMap.insert(std::make_pair("#5", 8));
    scaleDegreesMap.insert(std::make_pair("b6", 8));
    scaleDegreesMap.insert(std::make_pair("m6", 8));
    scaleDegreesMap.insert(std::make_pair("6", 9));
    scaleDegreesMap.insert(std::make_pair("M6", 9));
    scaleDegreesMap.insert(std::make_pair("#6", 10));
    scaleDegreesMap.insert(std::make_pair("b7", 10));
    scaleDegreesMap.insert(std::make_pair("m7", 10));
    scaleDegreesMap.insert(std::make_pair("7", 11));
    scaleDegreesMap.insert(std::make_pair("M7", 11));
}

int ScaleDegrees::getChromatic(std::string scaleDegreeName)
{
    if (scaleDegreesMap.size() == 0)
        init();
    
    int retVal = -1;
    std::map<std::string, int>::iterator i = scaleDegreesMap.find(scaleDegreeName);
    if (i != scaleDegreesMap.end())
        retVal = (*i).second;
    
    return retVal;
}