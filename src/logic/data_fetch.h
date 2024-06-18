#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "data_point.h"
#include "profil_fetch.h"


class DataFetch {
public:
    DataFetch();
    ~DataFetch();

public:
    bool data_ready = false;
    std::condition_variable cv_data_ready;

    void fetch(std::vector<std::string> profiles);
    void stop();
    void resume();
    void cancel();
    DataPoint* get_data_point(std::string profile);

private:
    bool m_stop_flag = false;
    bool m_cancel_flag = false;
    std::condition_variable m_cv_internal;
    std::map<std::string, DataPoint*> m_data_points;
    std::mutex m_mu;
    ProfileFetch* m_profile_fetch;
};
