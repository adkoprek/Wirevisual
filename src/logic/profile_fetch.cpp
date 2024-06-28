//  _    _ _                _                 _
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// A class to fetch all informations about a profile
//
// This class uses EPICS a library developed by Jan Chrin
// to comunicate with epics in debuging it can sometimes
// return the error code 400 but the data will still be loded
// just ignore it was the advise of the maker
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "data_point.h"
#include "epics_commands.h"
#include "profile_fetch.h"
#include "cafe.h"


/************************************************************
*                       public
************************************************************/

// Constructor
ProfileFetch::ProfileFetch() {
    m_cafe = new CAFE();
    m_cafe->channelOpenPolicy.setTimeout(1.0);  // Set timuout for cafe
}

ProfileFetch::~ProfileFetch() {
    m_cafe->closeChannels();
    delete m_cafe;
}

// Fethc a specific profile by name
int ProfileFetch::fetch(std::string profile_name, DataPoint* profile) {
    m_current_profile = profile_name;
    profile->name = profile_name;
    profile->valid_data = false;

#ifndef DEBUG
    // This code block activates a scan
    // It is advised to disable it with #define DEBUG
    // when not runing the code directly on the HIPA machine
    // because you are otherwise not allowed to write to the EPICS
    // server even on hipalc

    // If profile is a harp it doesn't have to be activated
    if (profile_name[2] != 'H') {
        activate_scan();
        char i = 0;

        // Wait for the scan to finish
        while (i < 50) {
            if (scan_finished())
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            i++;
        }

        // There was a timueout
        if (i > 50)
            return -1;
    }
#endif // !DEBUG


    // Fetch the direction of the profile
    auto direction = get_direction();
    if (direction == -2) {
        // That data is not required so it can be skipped
        std::cerr << "An error occured while fetching the direction of \"" << m_current_profile;
        std::cerr << "\" but continuing fetching of profile";
    }
    profile->direction = direction;

    auto size = get_size_of_profile();
    if (size == -1)
        return -2;

    // Define stard arrays because cafe only works with these
    double x_data[size];
    double y_data[size];

    // Fetche the data points
    auto return_code = load_profile_data(x_data, y_data);
    if (return_code != 0)
        return -3;

    // Rounding is advised
    round_data(size, x_data, y_data);

    // Convert the arras to standart vectors
    profile->x = std::vector<double>(x_data, x_data + size);
    profile->y = std::vector<double>(y_data, y_data + size);
    profile->valid_data = true;

    return 0;
}

/************************************************************
*                       private
************************************************************/

// Activate a wirescan
int ProfileFetch::activate_scan() {
    // For building PV's look up epics_commands.h
    std::string pv = m_current_profile + PV_ACTIVATE_SCAN;

    // Activate the scan with the START command
    int status = m_cafe->set(pv.c_str(), START_COMMAND.c_str());
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while starting the wirescan for the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }

    return 0;
}

// Check if wirescan has finished
bool ProfileFetch::scan_finished() {
    std::string profile_status;

    std::string command = m_current_profile + PROFILE_PV_STATUS;
    int status = m_cafe->get(command.c_str(), profile_status);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the size of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }

    // When a profile is in Idle mode it has finished
    if (profile_status == "Idle") return true;
    return false;

}

// Get the direction in which the scan was made
int ProfileFetch::get_direction() {
    int direction = 0;

    std::string pv = m_current_profile + PROFILE_PV_DIRECTION;
    int status = m_cafe->get(pv.c_str(), direction);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the size of the profile \"";
        std::cerr << m_current_profile << "\"" << std::endl;
        return -2;
    }

    return direction;
}

// Get the size of a profile
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

// Load the profiles x and y data
int ProfileFetch::load_profile_data(double* x_data, double* y_data) {
    std::vector<unsigned int> handles;
    std::string pv_x = m_current_profile + PROFILE_PV_X;
    std::vector<std::string> pvs_x = { pv_x };
    std::string pv_y;

    // A harp has a special PV for the y data
    if (m_current_profile[2] == 'H')
        pv_y = m_current_profile + HARP_PV_Y;
    else
        pv_y = m_current_profile + PROFILE_PV_Y;

    // You have to put your pv and handles into an array to work
    // nobody knows why, to solve contact Jan Chrin
    std::vector<std::string> pvs_y = { pv_y };

    // CAFE docs say you don't have to call open() but it only works
    // like this again to solve contact Jan Chrin
    m_cafe->open(pvs_x, handles);
    int status = m_cafe->get(pv_x.c_str(), x_data);
    m_cafe->closeHandlesV(handles);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the x data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -1;
    }

    m_cafe->open(pvs_y, handles);
    status = m_cafe->get(pv_y.c_str(), y_data);
    m_cafe->closeHandlesV(handles);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the y data of the profile \"" <<
            m_current_profile << "\"" << std::endl;
        return -2;
    }

    return 0;
}

// Data can be returned in scientific notation solved by rounding
void ProfileFetch::round_data(int16_t size, double* x_data, double* y_data) {
    for (int16_t i = 0; i < size; i++) {
        x_data[i] = std::round(x_data[i] * 1000000) / 1000000;
        y_data[i] = std::round(y_data[i] * 1000000) / 1000000;
    }
}
