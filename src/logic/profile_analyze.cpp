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

#include <cmath>
#include <vector>

#include "profile_analyze.h"

// Reference to the external fit function in fitting.cpp
extern int
FunctionFit(int fittype, float *XP, float *RP, int Nvals, float *estimation,
            float *result, float *error, float *RPout, float *chisqr, int *Nparam, char *fiterr);


/************************************************************
*                       public
************************************************************/

// Empty constructor definition
ProfileAnalyze::ProfileAnalyze() {}
ProfileAnalyze::~ProfileAnalyze() {}

// Analyze a profile and make some predicitons
void ProfileAnalyze::analyze(DataPoint* point) {
    m_x.clear();
    m_y.clear();
    m_point = point;
    m_size = m_point->x.size();
    m_point->offset = point->x[0];
    m_point->step = point->x[1] - point->x[0];

    // Do the actual mathematical prediction
    filter_data();
    find_maximum();
    find_interesting_positions();
    calc_sigma();
}

// Add the fit points for the profile passed in analyze()
std::vector<float> ProfileAnalyze::fit() {
    // Convert C++ variables to c styled variables
    float estimation[10], result[10], error[10];
    estimation[0] = m_peak;
    estimation[1] = m_centers[0];
    estimation[2] = m_sigma_total[0];
    estimation[3] = 0;
    int Nparam;
    float chisqr;
    char fiterr[128];
    std::vector<float> y_data(m_point->y.begin(), m_point->y.end());
    float fit_y[m_size];

    // Fit the points
    FunctionFit(2, m_x.data(), y_data.data(), m_y.size(), estimation, result, error, fit_y, &chisqr, &Nparam, fiterr);

    // If the offset was negativ fit again with other function
    if (result[3] < 0) {
        result[3] = 0;
        FunctionFit(7, m_x.data(), y_data.data(), m_y.size(), estimation, result, error, fit_y, &chisqr, &Nparam, fiterr);
    }

    // Add some calulcated result to the point
    m_point->mean_fit = m_point->offset + result[1] * m_point->step;
    m_point->sigma_4_fit = fabs(result[2] * m_point->step);

    // Fir function returns nan if the firt y-value is 0
    if (fit_y[0] != fit_y[0]) fit_y[0] = 0;

    return std::vector<float> { fit_y, fit_y + m_size };
}

/************************************************************
*                       private
************************************************************/

// Compare a data point with its neighbours and smooth it out
void ProfileAnalyze::filter_data() {
    m_y.push_back(m_point->y[0]);
    for (int i = 0; i < m_size; i++) {
        m_x.push_back(i);
        if ((i > 0) && ((i + 1) < m_size)) {
            double value = (m_point->y[i - 1] + 2 * m_point->y[i] + m_point->y[i + 1]) / 4;
            m_y.push_back(value);
        }
    }
    m_y.push_back(m_point->y[m_size - 1]);
}

// Find the max value in the profile
void ProfileAnalyze::find_maximum() {
    m_area = 0;
    m_max_index = 0;
    m_peak = 0;


    for (int i = 0; i < m_size; i++) {
        if (m_y[i] > m_peak) {
            m_peak = m_y[i];
            m_max_index = i;
        }
        m_area += m_y[i];
    }

    m_half_height = m_peak / 2;
    m_height_135 = m_peak * 0.135;
    m_area_80 = m_area * 0.8;
}

// Mezger called it like that
void ProfileAnalyze::find_interesting_positions() {
    double temp_point;
    bool half_height_found = false;
    bool height_135_found = false;

    for (int i = m_max_index; i > 0; i--) {
        temp_point = m_y[i];
        if (temp_point > m_half_height){
            if (!half_height_found) m_max_half_height_index_l = i;
        } else half_height_found = true;

        if (temp_point > m_height_135) {
            if (!height_135_found) m_max_height_135_index_l = i;
        } else height_135_found = false;

        m_area += temp_point;
    }


    half_height_found = false;
    height_135_found = false;

    for (int i = m_max_index; i < m_size; i++) {
        temp_point = m_y[i];
        if (temp_point > m_half_height) {
            if (!half_height_found) m_max_half_height_index_r = i;
        } else half_height_found = true;

        if (temp_point > m_height_135) {
            if (!height_135_found) m_max_height_135_index_r = i;
        } else height_135_found = true;

        m_area += temp_point;
    }

    m_area = m_y[m_max_index];
    int j = 1;
    for (int i = m_max_index; i < m_size; i++) {
        if (m_max_index + j > m_size - 2) break;
        if (m_max_index - j < 1) break;
        m_area = m_area + m_y[m_max_index + j] + m_y[m_max_index - j];
        if (m_area > m_area_80) break;
        j++;
    }
}

// Calculate a sigma prediction
void ProfileAnalyze::calc_sigma() {
    double width_half_height = m_max_half_height_index_r - m_max_half_height_index_l;
    double SB, SEM, SZM, EM, ZM;

    for (int i = 0; i < 2; i++) {
        int start, end;
        if (i == 0) {
            start = 0;
            end = m_size;
        }
        else {
            start = m_max_half_height_index_l - width_half_height;
            if (start < 0) start = 0;
            end = m_max_half_height_index_r + width_half_height;
            if (end > m_size) end = m_size;
        }

        SB = 0;
        SEM = 0;
        SZM = 0;

        for (int j = start; j < end; j++) {
            double temp_point = m_y[j];
            EM = j * temp_point;
            ZM = j * EM;
            SB += temp_point;
            SEM += EM;
            SZM += ZM;
        }

        if (SB > 0) {
            m_centers[i] = SEM / SB;
            m_sigma_total[i] = (SZM / SB) - std::pow(m_centers[i], 2);
            if (m_sigma_total[i] > 0) m_sigma_total[i] = std::sqrt(m_sigma_total[i]);
            else m_sigma_total[i] = 0;
        }
        else {
            m_centers[i] = 0;
            m_sigma_total[i] = 0;
        }
    }

    m_point->mean = m_point->offset + m_centers[0] * m_point->step;
    m_point->sigma_4 = m_sigma_total[0] * m_point->step;
    m_point->sigma_4_red = m_sigma_total[1] * m_point->step;
    m_point->fwhm = m_half_height * m_point->step;
    m_point->fwhm_fit = m_half_height * m_point->step;
}
