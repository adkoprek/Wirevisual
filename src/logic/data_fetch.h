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
    DataFetch();
    ~DataFetch();

public:
    bool data_ready = false;

    void fetch(std::vector<std::string> profiles);
    std::string get_current_profile();
    int load(std::string file_name);
    void stop();
    void resume();
    void cancel();
    bool was_canceled();
    bool point_exists(std::string profile);
    DataPoint* get_data_point(std::string profile);
    FileHeader* get_file_header();

private:
    bool m_stop_flag = false;
    bool m_last_canceled = false;
    bool m_cancel_flag = false;
    std::condition_variable m_cv_internal;
    std::map<std::string, DataPoint*> m_data_points;
    std::mutex m_mu;
    std::string m_current_profile;
    ProfileFetch* m_profile_fetch;
    ProfileAnalyze* m_profile_analyze;
    FileHeader* m_file_header = nullptr;
};
