//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Struct datatype to represent a quad
//
// This struct stores all the fetched data about a 
// quadrupole magnet short quad from epics
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <string>


typedef struct QuadPoint {
    bool valid_data;
    std::string name;

    signed int acs;             // Data in not unit
    double field;               // Field measured in kGauss
    double current;             // Currend threw the quads measured in Amps
} QuadPoint;
