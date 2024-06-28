//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Holds the names for the current monitors of every
// beamline
//
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <map>
#include <string>


const std::map<std::string, std::string> CURENTS = {
    {"b860",        "MWC2"},
    {"bce",         "MBC1"},
    {"bw2",         "MWC2"},
    {"bx1",         "MXC1"},
    {"bx2",         "MXC1"},
    {"ip2",         "MYC1"},
    {"iw2",         "MXC1"},
    {"pkbhe",       "MHC5"},
    {"pksinq",      "MHC6"},
    {"pktebhe",     "MHC5"},
    {"pktm",        "MHC1"},
    {"pktmte",      "MHC4"},
    {"sinq",        "MHC6"},
    {"ucn",         "MBC1"}
};
