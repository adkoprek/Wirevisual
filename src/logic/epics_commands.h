#pragma once
#include "string"
#include <string>


// Profiles and harps
const std::string PROFILE_PV_X = ":PROF:2:P";
const std::string PROFILE_PV_Y = ":PROF:2:I";
const std::string HARP_PV_Y = ":PROF:2";
const std::string PROFILE_PV_SIZE = ":PROF:2:P.NELM";
const std::string PROFILE_PV_STATUS = ":PSTE:1";
const std::string PV_ACTIVATE_SCAN = ":PCOM:1";
const std::string START_COMMAND = "START";

// Other data for files
const std::string QUADS_PV_ACS = ":SOL:1";
const std::string QUADS_PV_AMPS = ":IST:2";
const std::string QUADS_PV_FIELD = ":IST:3";
const std::string RADION_MONITOR_PV_AMP = ":IST:2";
