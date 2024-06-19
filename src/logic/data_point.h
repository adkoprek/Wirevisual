#pragma once

#include <string>
#include <vector>


typedef struct DataPoint {
    bool valid_data;
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
} DataPoint;
