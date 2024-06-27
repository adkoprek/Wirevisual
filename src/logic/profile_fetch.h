#pragma once
#include "cafe.h"
#include "data_point.h"
#include <cstdint>
#include <string>
#include <vector>


typedef std::vector<double> data_vector;

class ProfileFetch {
public:
    ProfileFetch();
    ~ProfileFetch();
    int fetch(std::string profile_name, DataPoint* profile);

private:
    CAFE* m_cafe;
    std::string m_current_profile;

    int activate_scan();
    bool scan_finished();
    int get_direction();
    int16_t get_size_of_profile();
    int load_profile_data(double* x_data, double* y_data);
    void round_data(int16_t size, double* x_data, double* y_data);
};
