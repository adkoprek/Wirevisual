//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Struct datatype to represent a .mes file header
//
// This struct stores all the fetched data about a 
// header of a .mes file and some calculated from 
// the header
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <string>
#include <vector>


typedef struct FileHeader {
    std::vector<std::string> profile_names; // Holds all the profile names included in file
    std::string date;                       // Holds the date in form <year>-<month>-<day>
    std::string time;                       // Holds the time in form <hour>:<minute>:<second>
    std::string current_device;             // Holds the name of hte current monitor
    double current;                         // Holds the scalar of the current
    std::string current_unit;               // Holds the unit of the current
    std::string beam_line;                  // Holds the name of the beamline
    std::string fit_option;                 // Holds the chosen fir option as a string
    int num_profiles;                       // Holds the number of priofiles
} FileHeader;
