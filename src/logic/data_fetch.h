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

#pragma once
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "file_header.h"
#include "profile_analyze.h"
#include "profile_fetch.h"


class DataFetch {
public:
    /************************************************************
    *                       functions
    ************************************************************/
    
    // Constructor
    DataFetch();

    // Deconstructor
    ~DataFetch();

    // Function to fetch new profiles
    // @param vector with profile names
    void fetch(std::vector<std::string> profiles);

    // Get name of the current profile that is fetched
    // @return current profile name
    std::string get_current_profile();

    // Load an .mes fieles content
    // @param string with the path to the file 
    // @return error code
    int load(std::string file_name);

    // Stop execution
    void stop();

    // Resume execution
    void resume();

    // cancel execution
    void cancel();

    // Check if last run was canceled
    // @return true if canceled
    bool was_canceled();

    // Check if point was fetched in last round
    // @return true if fetched
    bool point_exists(std::string profile);

    // Get a point by namespace MyNamespace
    // @param profile name
    // @return pointer to profile data
    DataPoint* get_data_point(std::string profile);

    // Get the last file header
    // @return nullptr if no fiel was loaded yet
    FileHeader* get_file_header();

    /************************************************************
    *                       fields
    ************************************************************/

    bool data_ready = false;            // Check if data is ready

private:
    /************************************************************
    *                       fields
    ************************************************************/

    bool m_stop_flag = false;           // Internal flag for stopping loop execution
    bool m_cancel_flag = false;         // Internal flag for canceling loop execution
    bool m_last_canceled = false;       // Flag that holds the state if last execution was canceled
    std::condition_variable m_cv_internal;  // Condition variable for stopping execution
    std::map<std::string, DataPoint*> m_data_points; // Map to store fetched profiles
    std::mutex m_mu;                    // Mutex for access to internal variables
    std::string m_current_profile;      // The name of the current profile being fetched
    ProfileFetch* m_profile_fetch;      // Instance of profile fetch to really detch the data
    ProfileAnalyze* m_profile_analyze;  // Instance of profile analyze to fit the data
    FileHeader* m_file_header = nullptr;    // Get the header of the last opened file
};
