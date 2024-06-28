//  _    _ _                _                 _
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Wrapper class around the fit function in fitting.cpp
//
// These class can make some prediction about the
// gaussian function fit parameters that would suit
// the profile the best and then pass them of the fit
// function that can create a better fit
//
// @Author: Adam Koprek
// @Inspired by: Anton Mezger
// @Maintainer: Jochem Snuverink

#pragma once
#include <vector>

#include "data_point.h"


class ProfileAnalyze {
public:
/************************************************************
*                       functions
************************************************************/

    // Constructor
    ProfileAnalyze();

    // Deconstructor
    ~ProfileAnalyze();

    // Analyze a profile and make some predicitons
    // @param pointer to the data point of the profile
    void analyze(DataPoint* point);

    // Add the fit points for the profile passed in analyze()
    // @return vector with fitted data points
    std::vector<float> fit();

private:
    /************************************************************
    *                       functions
    ************************************************************/

    // Compare a data point with its neighbours and smooth it out
    void filter_data();

    // Find the max value in the profile
    void find_maximum();

    // Mezger called it like that
    void find_interesting_positions();

    // Calculate a sigma prediction
    void calc_sigma();

    /************************************************************
    *                       fields
    ************************************************************/

    DataPoint* m_point;             // Holds the current data point
    int m_size;                     // Holds the size of the current point

    std::vector<float> m_x;         // Internal filtered x points of profile
    std::vector<float> m_y;         // Internal filtered y points of profile

    // Fields needed for calculation
    double m_area;
    double m_peak;
    int m_max_index;
    double m_height_135;
    double m_half_height;
    double m_area_80;
    int m_max_half_height_index_l = 0;
    int m_max_height_135_index_l = 0;
    int m_max_half_height_index_r = 0;
    int m_max_height_135_index_r = 0;
    std::vector<double> m_centers = { 0, 0 };
    std::vector<double> m_sigma_total = { 0, 0 };
};
