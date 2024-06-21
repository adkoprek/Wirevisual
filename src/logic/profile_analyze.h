#pragma once

#include "data_point.h"
#include <vector>
class ProfileAnalyze {
public:
    ProfileAnalyze();
    ~ProfileAnalyze();
    void analyze(DataPoint* point);
    std::vector<float> fit();

private:
    void filter_data();
    void find_maximum();
    void find_interesting_positions();
    void calc_sigma();

    DataPoint* m_point;
    int m_size;

    std::vector<float> m_x; 
    std::vector<float> m_y; 
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
