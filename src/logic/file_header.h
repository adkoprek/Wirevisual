#pragma once
#include <string>
#include <vector>


typedef struct FileHeader {
    std::vector<std::string> profile_names;
    std::string date;
    std::string time;
    std::string current_device;
    double current;
    std::string current_unit;
    std::string beam_line;
    std::string fit_option;
    int num_profiles;
} FileHeader;
