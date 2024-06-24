#include "data_dump.h"
#include "currents.h"
#include "data_fetch.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include "epics_commands.h"
#include "profiles.h"
#include "quad_fetch.h"
#include "quads.h"

#define FILE_BASE_PATH "."


DataDump::DataDump(DataFetch* data_fetch) {
    this->m_data_fetch = data_fetch;
    m_cafe = new CAFE;
    m_cafe->channelOpenPolicy.setTimeout(1.0);
    m_quad_fetch = new QuadFetch();
}

DataDump::~DataDump() {
    m_cafe->closeChannels();
    delete m_cafe;
    delete m_quad_fetch;
}

void DataDump::dump(std::vector<std::string> beam_lines, std::string fit) {
    m_fit = fit;

    for (int i = 0; i < beam_lines.size(); i++) {
        m_beam_line = beam_lines[i];
        m_mes_file = new std::ofstream(get_file_path());
        *m_mes_file << std::fixed << std::showpoint;
        fetch_current_profile_names(); 
        fetch_current();
        add_header();

        for (int i = 0; i < m_current_profile_names.size(); i++)
            add_profile_data(m_current_profile_names[i]);

        m_quad_fetch->fetch(QUADS.at(m_beam_line));
        add_quads();

        m_mes_file->close();
        delete m_mes_file;
    }
}

void DataDump::get_local_time() {
    time_t now = time(0);
    m_tm = localtime(&now);
    m_tm->tm_year += 1900;
    m_tm->tm_mon += 1;
}

std::string DataDump::get_file_path() {
    get_local_time();
    std::string file_name;
    file_name += m_beam_line + "_";
    file_name += std::to_string(m_tm->tm_year);
    if (m_tm->tm_mon < 9) file_name += "0";
    file_name += std::to_string(m_tm->tm_mon);
    if (m_tm->tm_mday < 9) file_name += "0";
    file_name += std::to_string(m_tm->tm_mday);
    file_name += "_";
    if (m_tm->tm_hour < 9) file_name += "0";
    file_name += std::to_string(m_tm->tm_hour);
    if (m_tm->tm_min < 9) file_name += "0";
    file_name += std::to_string(m_tm->tm_min);
    if (m_tm->tm_sec < 9) file_name += "0";
    file_name += std::to_string(m_tm->tm_sec);
    return std::string(FILE_BASE_PATH) + "/" + file_name + ".mes";
}

void DataDump::fetch_current_profile_names() {
    m_current_profile_names.clear();

    auto profiles = PROFILES.at(m_beam_line);

    for (int i = 0; i < profiles.size(); i++) {
        if (m_data_fetch->point_exists(profiles[i])) {
            if (m_data_fetch->get_data_point(profiles[i])->valid_data)
                m_current_profile_names.push_back(profiles[i]); 
        }
    }
}
    
int DataDump::fetch_current() {
    m_current_monitor = CURENTS.at(m_beam_line);
    std::string pv = m_current_monitor + CURRENT_PV_MONITOR;
    int status = m_cafe->get(pv.c_str(), m_current);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the current of \"";
        std::cerr << m_current_monitor << "\"" << std::endl;
        return -1;
    }

    PVCtrlHolder pvc;
    status = m_cafe->getCtrl(pv.c_str(), pvc);
    if (status != ICAFE_NORMAL) {
        std::cerr << "An error occured while fetching the unit of \"";
        std::cerr << m_current_monitor << "\"" << std::endl;
        return -2;
    }
    m_current_unit = pvc.getUnits();

    return 0;
}

void DataDump::add_header() {
    *m_mes_file << "date       ";
    if (m_tm->tm_mday < 9) *m_mes_file << "0";
    *m_mes_file << m_tm->tm_mday << "-";
    if (m_tm->tm_mon < 9) *m_mes_file << "0";
    *m_mes_file << m_tm->tm_mon << "-";
    *m_mes_file << m_tm->tm_year << " ";
    if (m_tm->tm_hour < 9) *m_mes_file << "0";
    *m_mes_file << m_tm->tm_hour << ":";
    if (m_tm->tm_min < 9) *m_mes_file << "0";
    *m_mes_file << m_tm->tm_min << ":";
    if (m_tm->tm_sec < 9) *m_mes_file << "0";
    *m_mes_file << m_tm->tm_sec << "\"" << std::endl;;

    *m_mes_file << "current    " << m_current_monitor << "   ";
    *m_mes_file << std::setprecision(1) << m_current;
    *m_mes_file << "   " << m_current_unit << std::endl;
    *m_mes_file << "beamline   " << m_beam_line << std::endl;
    *m_mes_file << "impuls     ??????   0.000000   \"\"" << std::endl;
    *m_mes_file << "option     " << m_fit << std::endl;
    *m_mes_file << "nbprofs    " << m_current_profile_names.size() << std::endl; 
}

void DataDump::add_profile_data(std::string profile) {
    auto point = m_data_fetch->get_data_point(profile);
    if (!point->valid_data) return;

    *m_mes_file << std::setprecision(3) << "Profil     " << profile;
    *m_mes_file << "   offset " << point->offset;
    *m_mes_file << "   step " << point->step;
    *m_mes_file << "   nb " << point->x.size();
    *m_mes_file << "   mean " << point->mean;
    *m_mes_file << "  meanFit " << point->mean_fit;
    *m_mes_file << " 4sigma " << point->sigma_4;
    *m_mes_file << " 4sigmaRed " << point->sigma_4_red;
    *m_mes_file << " 4sigmaFit " << point->sigma_4_fit;
    *m_mes_file << " fwhm " << point->fwhm;
    *m_mes_file << " fwhmFit " << point->mean_fit << std::endl;

    add_vector(point->x);
    add_vector(point->y);
    add_vector(point->fit);
}

void DataDump::add_vector(std::vector<double> data) {
    *m_mes_file << "           " << std::setprecision(6);
    for (int i = 0; i < data.size(); i++) 
        *m_mes_file << data[i] << " ";
    *m_mes_file << std::endl;
}

void DataDump::add_quads() {
    auto current_quads = QUADS.at(m_beam_line);

    *m_mes_file << "quaddacs   " << m_quad_fetch->get_num_valid_measurements() << std::endl;;   
    for (int i = 0; i < current_quads.size(); i++) {
        auto quad = m_quad_fetch->get_quad(current_quads[i]);
        if (!quad->valid_data) continue;

        *m_mes_file << quad->name << " " << quad->acs << "    ";
    }
    *m_mes_file << std::endl;

    *m_mes_file << std::setprecision(4);
    *m_mes_file << "quadfields " << m_quad_fetch->get_num_valid_measurements() << std::endl;;   
    for (int i = 0; i < current_quads.size(); i++) {
        auto quad = m_quad_fetch->get_quad(current_quads[i]);
        if (!quad->valid_data) continue;

        *m_mes_file << quad->name << " " << quad->field << "    ";
    }
    *m_mes_file << std::endl;

    *m_mes_file << "quadamps " << m_quad_fetch->get_num_valid_measurements() << std::endl;;   
    for (int i = 0; i < current_quads.size(); i++) {
        auto quad = m_quad_fetch->get_quad(current_quads[i]);
        if (!quad->valid_data) continue;

        *m_mes_file << quad->name << " " << quad->current << "    ";
    }
    *m_mes_file << std::endl;
}
