#include "data_fetch.h"
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <vector>
#include "file_header.h"
#include "data_point.h"
#include "profile_analyze.h"
#include "profile_fetch.h"


// original methods from caMifQt
namespace {
  /// Get next line
  std::string getNextLine(std::ifstream& file)
  {
    std::string line;
    while (file.good()) {
      std::getline(file,line);
      if (!line.empty() && (line.front() != '#'))
        {break;}
    }
    return line;
  }
  /// Break line into pieces, convert to doubles and store in vector
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

int DataFetch::load(std::string file_name) {
    std::ifstream file;
    std::string dummy;
    file.open(file_name, std::ios_base::in);
    if (!file.is_open()) {
        std::cerr << "Error opening config \"" << file_name << "\"" << std::endl;
        return -1;
    }


    for (const auto & [key, value] : m_data_points)
        delete value;
    m_data_points.clear();

    delete m_file_header;
    m_file_header = new FileHeader();

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
             >> dummy >> point->fwhm_fit;

        if (point->name.length() < 5) point->name.insert(3, "0");
        m_file_header->profile_names.push_back(point->name);

        if (point->nb != 0) {
            getNextVector(file, point->x);
            getNextVector(file, point->y);
            getNextVector(file, point->fit);
        }
        m_data_points.insert({point->name, point});
    }

    return 0;
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

FileHeader* DataFetch::get_file_header() {
    return m_file_header;
}
