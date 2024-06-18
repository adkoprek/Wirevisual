#include "data_fetch.h"
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <vector>
#include "profile_fetch.h"


DataFetch::DataFetch() {
    m_profile_fetch = new ProfileFetch();
}

DataFetch::~DataFetch() {
    delete m_profile_fetch;
}

void DataFetch::fetch(std::vector<std::string> profiles) {
    data_ready = false;

    for (size_t i = 0; i < profiles.size(); i++) {
        if (m_stop_flag) {
            std::unique_lock<std::mutex> lock(m_mu);
            m_cv_internal.wait(lock, [this]{ return !m_stop_flag; });
        }

        if (m_cancel_flag) return;

        DataPoint* profile = new DataPoint();
        int return_code = m_profile_fetch->fetch(profiles[i], profile);
        if (return_code != 0)
            std::cerr << "An error occured while fetching profile \"" << profiles[i] << 
                "\", the function exited with error code " << return_code << std::endl;
        if (m_data_points.count(profiles[i]))
            m_data_points[profiles[i]] = profile;
        else
            m_data_points.insert({ profiles[i], profile });
    }

    data_ready = true;
    cv_data_ready.notify_all();
}

void DataFetch::stop() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_stop_flag = true;
}

void DataFetch::resume() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_stop_flag = false;;
    m_cv_internal.notify_one();
}


void DataFetch::cancel() {
    std::lock_guard<std::mutex> lock(m_mu); 
    m_cancel_flag = true;
}

DataPoint* DataFetch::get_data_point(std::string id) {
    if (m_data_points.count(id)) return m_data_points[id];
    return nullptr;
}
