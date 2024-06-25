#pragma once
#include "cafe.h"
#include "data_fetch.h"
#include "fits.h"
#include "quad_fetch.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>


typedef std::vector<std::string> str_array;
typedef char str9[9];
typedef char str50[50];

class DataDump {
public:
    DataDump(DataFetch* data_fetch);
    ~DataDump();

    void dump(str_array beam_lines, FITS fit);
    
private:
    CAFE* m_cafe;
    FITS m_fit;
    DataFetch* m_data_fetch;
    QuadFetch* m_quad_fetch;
    str_array m_current_profile_names;
    double m_current;
    std::string m_current_monitor;
    std::string m_current_unit;
    std::ofstream* m_mes_file;
    std::ofstream* m_022_file;
    struct std::tm* m_tm;
    std::string m_beam_line;
    std::string m_date;

    // Functions to fetch some data
    void get_local_time();
    std::string get_file_path();
    void fetch_current_profile_names();
    int fetch_current();

    // Functions for creating .mes file
    void add_mes_header();
    void add_mes_profile_data(std::string profile);
    void add_mes_vector(std::vector<double> data);
    void add_mes_quads();

    // Functions to create transport files
    void create_transport_file();

    // Function for creating .022 file
    void add_022_quads();
    void add_022_sigmah();
    void add_022_sigmav();
};
