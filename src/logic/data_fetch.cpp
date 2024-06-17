#include "data_fetch.h"
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <cstddef>
#include <map>
#include <mutex>
#include <sys/types.h>
#include <thread>
#include <vector>


DataFetch::DataFetch() {}
DataFetch::~DataFetch() {}

void DataFetch::fetch(std::vector<uint8_t> ids) {
    data_ready = false;

    for (size_t i = 0; i < ids.size(); i++) {
        if (m_stop_flag) {
            std::unique_lock<std::mutex> lock(m_mu);
            m_cv_internal.wait(lock, [this]{ return !m_stop_flag; });
        }

        if (m_cancel_flag) return;

        std::cout << "Fethcing: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (!m_data_points.count(i)) 
            m_data_points.insert({ i, new DataPoint() });

        else m_data_points[i] = new DataPoint();
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

DataPoint* DataFetch::get_data_point(int id) {
    if (m_data_points.count(id)) return m_data_points[id];
    return nullptr;
}
