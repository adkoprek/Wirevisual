#pragma once

#include <condition_variable>
#include <cstdint>
#include <map>
#include <mutex>
#include <vector>
#include "data_point.h"


class DataFetch {
public:
    DataFetch();
    ~DataFetch();

public:
    bool data_ready = false;
    std::condition_variable cv_data_ready;

    void fetch(std::vector<uint8_t> ids);
    void stop();
    void resume();
    void cancel();
    DataPoint* get_data_point(int id);

private:
    bool m_stop_flag = false;
    bool m_cancel_flag = false;
    std::condition_variable m_cv_internal;
    std::map<uint8_t, DataPoint*> m_data_points;
    std::mutex m_mu;
};
