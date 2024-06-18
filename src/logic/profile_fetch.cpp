#include "data_point.h"
#include "epics_commands.h"
#include "profil_fetch.h"
#include "cafe.h"
#include <cstdint>
#include <iostream>
#include <string>


ProfileFetch::ProfileFetch() {
    m_cafe = new CAFE();
}

ProfileFetch::~ProfileFetch() {
    delete m_cafe;
}

int ProfileFetch::fetch(std::string profile_name, DataPoint* profile) {
    m_current_profile = profile_name;
    auto size = get_size_of_profile(); 
    if (size == -1) return -1;
        
    data_vector* x_data;
    data_vector* y_data;
    auto return_code = load_profile_data(size, x_data, y_data);
    if (return_code != 0) return -2;

    profile->name = profile_name;
    profile->x = x_data;
    profile->y = y_data;

    return 0;
}

int16_t ProfileFetch::get_size_of_profile() {
    int16_t size;

    std::string pv = m_current_profile + PROFILE_PV_SIZE;
    int status = m_cafe->get(pv.c_str(), size);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the size of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }
    return size;
}

int ProfileFetch::load_profile_data(int16_t size, data_vector* x_data, data_vector* y_data) {
    float x_data_buf[size];
    float y_data_buf[size];

    std::string command_x = m_current_profile + PROFILE_PV_X;
    std::string command_y = m_current_profile + PROFILE_PV_Y;
    int status = m_cafe->get(command_x.c_str(), x_data_buf);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the x data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }
    status = m_cafe->get(command_y.c_str(), y_data_buf);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the y data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -2;
    }

    x_data = new data_vector(x_data_buf, x_data_buf + size);
    y_data = new data_vector(y_data_buf, y_data_buf + size);

    return 0;
}
