#include "data_point.h"
#include "epics_commands.h"
#include "profile_fetch.h"
#include "cafe.h"
#include <boost/numeric/conversion/detail/meta.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>


ProfileFetch::ProfileFetch() {
    m_cafe = new CAFE();
    m_cafe->channelOpenPolicy.setTimeout(5.0);
}

ProfileFetch::~ProfileFetch() {
    m_cafe->closeChannels();
    delete m_cafe;
}

int ProfileFetch::fetch(std::string profile_name, DataPoint* profile) {
    m_current_profile = profile_name;

#ifdef PRODUCTION 
    #include <chrono>
    #include <thread>
    
    activate_scan();
    while (scan_finished())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif // PRODUCTION
    

    auto size = get_size_of_profile(); 
    if (size == -1) return -1;
        
    double x_data[size];
    double y_data[size];
    auto return_code = load_profile_data(x_data, y_data);
    std::cout << "Some data: " << x_data[399] << std::endl;
    if (return_code != 0) return -2;

    profile->name = profile_name;
    profile->x = std::vector<double>(x_data, x_data + size);
    profile->y = std::vector<double>(y_data, y_data + size);

    return 0;
}

int ProfileFetch::activate_scan() {
    std::string command = m_current_profile + PV_ACTIVATE_SCAN;

    int status = m_cafe->set(m_current_profile.c_str(), START_COMMAND.c_str());
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while starting the wirescan for the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }

    return 0;
}

bool ProfileFetch::scan_finished() {
    std::string profile_status;

    std::string command = m_current_profile + PROFILE_PV_STATUS;
    int status = m_cafe->get(command.c_str(), status);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the size of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }

    if (profile_status == "Idle") return true;
    return false;
    
}

int16_t ProfileFetch::get_size_of_profile() {
    int16_t size;

    std::string pv = m_current_profile + PROFILE_PV_SIZE;
    int status = m_cafe->get(pv.c_str(), size);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the size of the profile \"";
        std::cerr << m_current_profile << "\"" << std::endl;
        return -1;
    }

    return size;
}

int ProfileFetch::load_profile_data(double* x_data, double* y_data) {
    std::vector<unsigned int> handles;


    std::string command_x = m_current_profile + PROFILE_PV_X;
    std::vector<std::string> pvs_x = { command_x };
    std::string command_y = m_current_profile + PROFILE_PV_Y;
    std::vector<std::string> pvs_y = { command_y };

    m_cafe->open(pvs_x, handles);
    int status = m_cafe->get(command_x.c_str(), x_data);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the x data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }
    m_cafe->open(pvs_y, handles);
    status = m_cafe->get(command_y.c_str(), y_data);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the y data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -2;
    }

    return 0;
}
