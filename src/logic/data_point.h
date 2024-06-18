#pragma once

#include <string>
#include <vector>


typedef struct DataPoint {
    std::string name;
    std::vector<float>* x;
    std::vector<float>* y;
} DataPoint;
