//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Class interface for an interactive thread
//
// This class is used to fetch all the data for a vector of
// profile names while being able to stop, resume and cancel
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <vector>

#include "data_fetch.h"
#include "file_header.h"
#include "data_point.h"
#include "profile_analyze.h"
#include "profile_fetch.h"


// These are external helper methods copied from caMifQt
namespace {
    /// Get next line
    std::string getNextLine(std::ifstream& file)
    {
        std::string line;
        while (file.good()) {
            std::getline(file,line);
            if (!line.empty() && (line.front() != '#'))
                break;
        }
        return line;
    }

    /// Break line into pieces, convert to doubles and store in vectorvoid
    void streamLine(const std::string& line, std::vector<double>& data)
    {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            try {
                double value = std::stod(word);
                data.push_back(value);
            } catch (std::invalid_argument&) {
                std::cerr << "Not a valid number " << word << std::endl;
            }
        }
    }

    template <typename T>
    void getNextVector(std::ifstream& file, std::vector<T>& data)
    {
        std::string line = getNextLine(file);
        streamLine(line,data);
    }
}

/************************************************************
*                       public
************************************************************/

DataFetch::DataFetch() {
    m_profile_fetch = new ProfileFetch();
    m_profile_analyze = new ProfileAnalyze();
}

DataFetch::~DataFetch() {
    for (const auto & [key, value] : m_data_points)
        delete value;

    delete m_profile_fetch;
    delete m_profile_analyze;
    delete m_file_header;
}

// Function to fetch new profiles
void DataFetch::fetch(std::vector<std::string> profiles) {
    data_ready = false;
    m_last_canceled = false;

    // Clear all the old data points
    for (const auto & [key, value] : m_data_points)
        delete value;
    m_data_points.clear();

    for (size_t i = 0; i < profiles.size(); i++) {
        // If stop flag set then wait for the cv to be unlocked by resume()
        if (m_stop_flag) {
            std::unique_lock<std::mutex> lock(m_mu);
            m_cv_internal.wait(lock, [this]{ return !m_stop_flag; });
        }

        // If cancel falg set by cancel() return
        if (m_cancel_flag) {
            m_cancel_flag = false;
            m_last_canceled = true;
            return;
        }

        // Create a new profile
        m_current_profile = profiles[i];
        DataPoint* profile = new DataPoint();
        int return_code = m_profile_fetch->fetch(m_current_profile, profile);
        m_data_points.insert({ m_current_profile, profile });
        if (return_code != 0) {
            std::cerr << "An error occured while fetching profile \"" << profiles[i];
            std::cerr << "\", the function exited with error code " << return_code << std::endl;
            continue;
        }

        // Fit a gausian to the profile
        m_profile_analyze->analyze(profile);
        std::vector<float> fit_y = m_profile_analyze->fit();
        profile->fit = std::vector<double>(fit_y.begin(), fit_y.end());
    }

    data_ready = true;
}

// Get name of the current profile that is fetched
std::string DataFetch::get_current_profile() {
    return m_current_profile;
}

// Load an .mes fieles content
// @Inspired by: Jochem Snuverink
int DataFetch::load(std::string file_name) {
    std::ifstream file;
    std::string dummy;
    file.open(file_name, std::ios_base::in);
    if (!file.is_open()) {
        std::cerr << "Error opening config \"" << file_name << "\"" << std::endl;
        return -1;
    }

    // Clear all old datapoints
    for (const auto & [key, value] : m_data_points)
        delete value;
    m_data_points.clear();

    delete m_file_header;
    m_file_header = new FileHeader();

    // Read the header of the .mes file
    file >> dummy >> m_file_header->date >> m_file_header->time;
    m_file_header->date.erase(std::remove(m_file_header->date.begin(), m_file_header->date.end(), '"'), m_file_header->date.end());
    m_file_header->time.erase(std::remove(m_file_header->time.begin(), m_file_header->time.end(), '"'), m_file_header->time.end());
    file >> dummy >> m_file_header->current_device >> m_file_header->current >> m_file_header->current_unit;
    file >> dummy >> m_file_header->beam_line;
    std::getline(file, dummy);
    std::getline(file, dummy);
    file >> dummy >> m_file_header->fit_option;
    file >> dummy >> m_file_header->num_profiles;
    std::getline(file, dummy);

    // Read every profile of the .mes file
    for (int i = 0; i < m_file_header->num_profiles; i++) {
        DataPoint* point = new DataPoint();
        point->valid_data = true;
        file >> dummy >> point->name
             >> dummy >> point->offset
             >> dummy >> point->step
             >> dummy >> point->nb
             >> dummy >> point->mean
             >> dummy >> point->mean_fit
             >> dummy >> point->sigma_4
             >> dummy >> point->sigma_4_red
             >> dummy >> point->sigma_4_fit
             >> dummy >> point->fwhm
             >> dummy >> point->fwhm_fit
             >> dummy >> point->direction;

        if (point->name.length() < 5) point->name.insert(3, "0");
        m_file_header->profile_names.push_back(point->name);

        // Read the individuals data points
        if (point->nb != 0) {
            getNextVector(file, point->x);
            getNextVector(file, point->y);
            getNextVector(file, point->fit);
        }

        m_data_points.insert({point->name, point});
    }

    // The quadrupoles are not read

    return 0;
}

// Stop execution
void DataFetch::stop() {
    std::lock_guard<std::mutex> lock(m_mu);         // Meant to be run in a thread
    m_stop_flag = true;
}

// Resume execution
void DataFetch::resume() {
    std::lock_guard<std::mutex> lock(m_mu);         // Meant to be run in a thread
    m_stop_flag = false;
    m_cv_internal.notify_one();                     // Notify the main execution it can continue
}

// cancel execution
void DataFetch::cancel() {
    std::lock_guard<std::mutex> lock(m_mu);         // Meant to be run in a thread
    m_cancel_flag = true;
}

// Check if last run was canceled
bool DataFetch::was_canceled() {
    return m_last_canceled;
}

// Check if point was fetched in last round
bool DataFetch::point_exists(std::string profile) {
    return m_data_points.count(profile);
}

// Get a point by namespace MyNamespace
DataPoint* DataFetch::get_data_point(std::string id) {
    if (m_data_points.count(id)) return m_data_points[id];
    return nullptr;
}

// Get the last file header
FileHeader* DataFetch::get_file_header() {
    return m_file_header;
}
