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

#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "cafe.h"
#include "data_point.h"

// Define a type shortening
typedef std::vector<double> data_vector;


class ProfileFetch {
public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    ProfileFetch();

    // Destructor
    ~ProfileFetch();

    // Fethc a specific profile by name
    // @param the name of the profile
    // @param pointer the the profile into which fetch the data
    int fetch(std::string profile_name, DataPoint* profile);

private:
    /************************************************************
    *                       functions
    ************************************************************/

    // Activate a wirescan
    // @return error code
    int activate_scan();

    // Check if wirescan has finished
    // @return true if finished
    bool scan_finished();

    // Get the direction in which the scan was made
    // @return -2 means error
    int get_direction();

    // Get the size of a profile
    // @return -1 means error
    int16_t get_size_of_profile();

    // Load the profiles x and y data
    // @param pointer to an array for the x data
    // @param pointer to an array for the y data
    // @return error code
    int load_profile_data(double* x_data, double* y_data);

    // Data can be returned in scientific notation solved by rounding
    // @param number of points
    // @param pointer to an array with the x data
    // @param pointer to an array with the y data
    void round_data(int16_t size, double* x_data, double* y_data);

    /************************************************************
    *                       fields
    ************************************************************/

    CAFE* m_cafe;                   // Pointer the internal cafe instance for EPICS access
    std::string m_current_profile;  // The name of the currently fetched profile
};
