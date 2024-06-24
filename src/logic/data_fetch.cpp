#include "data_fetch.h"
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <vector>
#include "profile_analyze.h"
#include "profile_fetch.h"


DataFetch::DataFetch() {
    m_profile_fetch = new ProfileFetch();
    m_profile_analyze = new ProfileAnalyze();
}

DataFetch::~DataFetch() {
    for (const auto & [key, value] : m_data_points)
        delete value;

    delete m_profile_fetch;
    delete m_profile_analyze;
}

void DataFetch::fetch(std::vector<std::string> profiles) {
    data_ready = false;
    m_last_canceled = false;

    for (const auto & [key, value] : m_data_points)
        delete value;

    m_data_points.clear();

    for (size_t i = 0; i < profiles.size(); i++) {
        if (m_stop_flag) {
            std::unique_lock<std::mutex> lock(m_mu);
            m_cv_internal.wait(lock, [this]{ return !m_stop_flag; });
        }

        if (m_cancel_flag) {
            m_cancel_flag = false;
            m_last_canceled = true;
            return;
        }

        DataPoint* profile = new DataPoint();
        int return_code = m_profile_fetch->fetch(profiles[i], profile);
        m_data_points.insert({ profiles[i], profile });
        if (return_code != 0) {
            std::cerr << "An error occured while fetching profile \"" << profiles[i];
            std::cerr << "\", the function exited with error code " << return_code << std::endl;
            continue;
        }

        m_profile_analyze->analyze(profile);
        std::vector<float> fit_y = m_profile_analyze->fit();
        profile->fit = std::vector<double>(fit_y.begin(), fit_y.end());
    }

    data_ready = true;
}

void DataFetch::stop() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_stop_flag = true;
}

void DataFetch::resume() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_stop_flag = false;
    m_cv_internal.notify_one();
}

void DataFetch::cancel() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_cancel_flag = true;
}

bool DataFetch::was_canceled() {
    return m_last_canceled;
}

bool DataFetch::point_exists(std::string profile) {
    return m_data_points.count(profile);
}

DataPoint* DataFetch::get_data_point(std::string id) {
    if (m_data_points.count(id)) return m_data_points[id];
    return nullptr;
}
