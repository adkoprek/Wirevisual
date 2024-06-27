#pragma once

#include <string>
#include <vector>


typedef struct DataPoint {
    bool valid_data;
    std::string name;

    double offset;
    double step;
    double nb;
    double mean;
    double mean_fit;
    double sigma_4;
    double sigma_4_fit;
    double sigma_4_red;
    double fwhm;
    double fwhm_fit;
    int direction;

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> fit;
} DataPoint;
