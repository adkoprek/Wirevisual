#include "profile_analyze.h"
#include <cmath>
#include <vector>


extern int
FunctionFit(int fittype, float *XP, float *RP, int Nvals, float *estimation,
            float *result, float *error, float *RPout, float *chisqr, int *Nparam, char *fiterr);


ProfileAnalyze::ProfileAnalyze() {}
ProfileAnalyze::~ProfileAnalyze() {}

void ProfileAnalyze::analyze(DataPoint* point) {
    m_x.clear();
    m_y.clear();
    m_point = point;
    m_size = m_point->x.size();

    filter_data();
    find_maximum();
    find_interesting_positions();
}

std::vector<float> ProfileAnalyze::fit() {
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
    // Convert to float vector
    FunctionFit(2, m_x.data(), y_data.data(), m_y.size(), estimation, result, error, fit_y, &chisqr, &Nparam, fiterr);
    
    if (result[3] < 0) {
        result[3] = 0;
        FunctionFit(7, m_x.data(), y_data.data(), m_y.size(), estimation, result, error, fit_y, &chisqr, &Nparam, fiterr);
    }


    return std::vector<float> { fit_y, fit_y + m_size };
}

void ProfileAnalyze::filter_data() {
    m_y.push_back(m_point->y[0]);
    for (int i = 0; i < m_size; i++) {
        m_x.push_back(i);
        if ((i > 0) && ((i + 1) < m_size)) {
            double value = (m_point->x[i - 1] + 2 * m_point->x[i] + m_point->x[i + 1]) / 4;
            m_y.insert(m_y.begin() + i, value);
        }
    }
    m_y.push_back(m_point->y[m_size - 1]);
}

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
            double temp_point = m_y[i];
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
}
