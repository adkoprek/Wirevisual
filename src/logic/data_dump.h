#pragma once
#include "cafe.h"
#include "data_fetch.h"
#include "quad_fetch.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>


typedef std::vector<std::string> str_array;

class DataDump {
public:
    DataDump(DataFetch* data_fetch);
    ~DataDump();

    void dump(str_array beam_lines, std::string fit);
    
private:
    CAFE* m_cafe;
    DataFetch* m_data_fetch;
    QuadFetch* m_quad_fetch;
    str_array m_current_profile_names;
    double m_current;
    std::string m_current_monitor;
    std::string m_current_unit;
    std::string m_fit;
    std::ofstream* m_mes_file;
    struct std::tm* m_tm;
    std::string m_beam_line;

    void get_local_time();
    std::string get_file_path();
    void fetch_current_profile_names();
    int fetch_current();
    void add_header();
    void add_profile_data(std::string profile);
    void add_vector(std::vector<double> data);
    void add_quads();
};
