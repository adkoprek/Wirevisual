#pragma once

#include <string>


typedef struct QuadPoint {
    bool valid_data;
    std::string name;

    signed int acs;
    double field;
    double current;
} QuadPoint;
