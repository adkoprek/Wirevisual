//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Constant values of the PV's for the EPICS server
//
// Every PV has to be combined with the name of the 
// thing you want to measure like
// MHP11 (profile monitor) + PROFILE_PV_X
// You can also use these PV's with a cli
//  - get: caget + PV
//  - info: cainfo + PV
//  - put: caput + PV (restricted to real machine)
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <string>


// Profiles and harps
const std::string PROFILE_PV_X = ":PROF:2:P";           // Get X-data of profile or harp
const std::string PROFILE_PV_Y = ":PROF:2:I";           // Get Y-data of profile
const std::string HARP_PV_Y = ":PROF:2";                // Get Y-data from harp
const std::string PROFILE_PV_SIZE = ":PROF:2:P.NELM";   // Number of points for harp or profile
const std::string PROFILE_PV_STATUS = ":PSTE:1";        // Get the status of the measurement
const std::string PV_ACTIVATE_SCAN = ":PCOM:1";         // Wrhite START to this pv to start measurement
const std::string START_COMMAND = "START";              // Command needed to start measurement
const std::string PROFILE_PV_DIRECTION = ":DIR:1";      // Get the direction the profile was measured in

// Quads
const std::string QUADS_PV_ACS = ":SOL:1";              // Get the acs of a quad
const std::string QUADS_PV_AMPS = ":IST:2";             // Get the amps of a quad
const std::string QUADS_PV_FIELD = ":IST:3";            // Get the field of a quad
                                                        
// Current monitors
const std::string CURRENT_PV_MONITOR = ":IST:2";        // Get the current of a current monitor
