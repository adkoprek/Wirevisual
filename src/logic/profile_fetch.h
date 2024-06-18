#pragma once
#include "cafe.h"
#include "data_point.h"
#include <cstdint>
#include <string>
#include <vector>


typedef std::vector<float> data_vector;

class ProfileFetch {
public:
    ProfileFetch();
    ~ProfileFetch();
    int fetch(std::string profile_name, DataPoint* profile);

private:
    CAFE* m_cafe;
    std::string m_current_profile;

    int activate_scan();
    int16_t get_size_of_profile();
    int load_profile_data(int16_t size, data_vector* x_data, data_vector* y_data);
};
