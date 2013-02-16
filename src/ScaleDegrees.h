#ifndef h_ScaleDegrees
#define h_ScaleDegrees

#include <iostream>
#include <map>

// Helper class to convert between diatonic and chromatic
class ScaleDegrees
{
public:
    // Get by name e.g. "b5", "4", "#6"
    static int getChromatic(std::string scaleDegreeName);
private:
    static void init();
    static std::map<std::string, int> scaleDegreesMap;
};

#endif // h_ScaleDegrees
