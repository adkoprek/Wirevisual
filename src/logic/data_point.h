//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Struct datatype to represent a profile measurement
//
// This struct stores all the fetched data about a 
// profile from epics but also the calculated data
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuvernik

#pragma once
#include <string>
#include <vector>


typedef struct DataPoint {
    bool valid_data;            // If there was a error while fetching
    std::string name;

    // Data calculated while fitting to Gauss
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

    // The points to be displayed
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> fit;
} DataPoint;
