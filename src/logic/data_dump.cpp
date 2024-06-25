#include "data_dump.h"
#include "currents.h"
#include "data_fetch.h"
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include "epics_commands.h"
#include "fits.h"
#include "profiles.h"
#include "quad_fetch.h"
#include "quads.h"

#define FILE_BASE_PATH "."
extern int updateTransportFile(char *transLine, int nbDevs, str9 *Quads, int *QuadsSign, float *values,
                        str50 *qerrMsg, int nbProfs, str9 *Profs, str50 *perrMsg, float *sigma2,
                        char *broDev, char *broUnit, float broVal, char *actualTime,
                        char *fileName, char *message,
                        int *nbHor, float *sigma2h, int *nbVer, float *sigma2v);



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

void DataDump::dump(std::vector<std::string> beam_lines, FITS fit) {
    m_fit = fit;
    get_local_time();

    for (int i = 0; i < beam_lines.size(); i++) {
        m_beam_line = beam_lines[i];
        auto file_path = get_file_path();
        m_mes_file = new std::ofstream(file_path + ".mes");
        m_022_file = new std::ofstream(file_path + ".022");

        // Fetch data
        fetch_current_profile_names(); 
        int status = fetch_current();
        if (status != 0) {
            std::cerr << "The previous error is not acceptable skipping beamline \"";
            std::cerr << m_beam_line << "\"" << std::endl;;
            continue;
        }

        // Create .mes file
        *m_mes_file << std::fixed << std::showpoint;
        add_mes_header();
        for (int i = 0; i < m_current_profile_names.size(); i++)
            add_mes_profile_data(m_current_profile_names[i]);

        m_quad_fetch->fetch(QUADS.at(m_beam_line));
        add_mes_quads();

        // Create all the transport files
        create_transport_file();

        // Create .022 file
        *m_022_file << std::setw(10) << std::fixed << std::setprecision(1);
        add_022_quads();
        add_022_sigmah();
        add_022_sigmav();

        m_mes_file->close();
        m_022_file->close();
        delete m_mes_file;
        delete m_022_file;
    }
}

void DataDump::get_local_time() {
    time_t now = time(0);
    m_tm = localtime(&now);
    m_tm->tm_year += 1900;
    m_tm->tm_mon += 1;
}

std::string DataDump::get_file_path() {
    std::string file_name;

    m_date = "";
    m_date += std::to_string(m_tm->tm_year);
    if (m_tm->tm_mon < 9) m_date += "0";
    m_date += std::to_string(m_tm->tm_mon);
    if (m_tm->tm_mday < 9) m_date += "0";
    m_date += std::to_string(m_tm->tm_mday);
    m_date += "_";
    if (m_tm->tm_hour < 9) m_date += "0";
    m_date += std::to_string(m_tm->tm_hour);
    if (m_tm->tm_min < 9) m_date += "0";
    m_date += std::to_string(m_tm->tm_min);
    if (m_tm->tm_sec < 9) m_date += "0";
    m_date += std::to_string(m_tm->tm_sec);

    file_name += m_beam_line + "_" + m_date;
    return std::string(FILE_BASE_PATH) + "/" + file_name;
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

void DataDump::add_mes_header() {
    *m_mes_file << "date       \"";
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
    *m_mes_file << "option     " << FIT_NAMES.at(m_fit) << std::endl;
    *m_mes_file << "nbprofs    " << m_current_profile_names.size() << std::endl; 
}

void DataDump::add_mes_profile_data(std::string profile) {
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

    add_mes_vector(point->x);
    add_mes_vector(point->y);
    add_mes_vector(point->fit);
}

void DataDump::add_mes_vector(std::vector<double> data) {
    *m_mes_file << "           " << std::setprecision(6);
    for (int i = 0; i < data.size(); i++) 
        *m_mes_file << data[i] << " ";
    *m_mes_file << std::endl;
}

void DataDump::add_mes_quads() {
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

void DataDump::create_transport_file() {
    auto current_quads = QUADS.at(m_beam_line);
    int num_profiles = m_current_profile_names.size();
    float sigma_2[num_profiles], sigma_2_h[num_profiles], sigma_2_v[num_profiles];
    str9 quads_arr[current_quads.size()], profs_arr[m_current_profile_names.size()];
    str50 querrMsg[current_quads.size()], perrMsg[current_quads.size()];
    float values[current_quads.size()];
    int quad_sign[current_quads.size()];
    char message[128] = { "" };
    int nb_hor = 0, nb_ver = 0;
    std::string file_path = get_file_path();
    for (int i = 0; i < current_quads.size(); i++) {
        strcpy((char*)&quads_arr[i], current_quads[i].c_str());
        values[i] = m_quad_fetch->get_quad(current_quads[i])->field;
        if (m_beam_line == "pktebhe") quad_sign[i] = -1;
        else quad_sign[i] = 1;
    }

    for (int i = 0; i < m_current_profile_names.size(); i++) {
        strcpy((char*)&profs_arr[i], m_current_profile_names[i].c_str());
        if (m_fit == FITS::TWO_SIGMA) 
            sigma_2[i] = 2 * m_data_fetch->get_data_point(m_current_profile_names[i])->sigma_4;
        else if (m_fit == FITS::TWO_SIGMA_RED)
            sigma_2[i] = 2 * m_data_fetch->get_data_point(m_current_profile_names[i])->sigma_4_red;
        else if (m_fit == FITS::TWO_SIGMA_FIT)
            sigma_2[i] = 2 * m_data_fetch->get_data_point(m_current_profile_names[i])->sigma_4_fit;
        else if (m_fit == FITS::FWHM)
            sigma_2[i] = m_data_fetch->get_data_point(m_current_profile_names[i])->fwhm;
        else if (m_fit == FITS::FWHM_FIT)
            sigma_2[i] = m_data_fetch->get_data_point(m_current_profile_names[i])->fwhm_fit;
        else sigma_2[i] = 999;
    }

    updateTransportFile((char*)m_beam_line.c_str(), current_quads.size(), quads_arr, quad_sign, 
                        values, querrMsg, num_profiles, profs_arr, perrMsg, sigma_2, (char*)"??????", 
                        (char*)"\0", 0, (char*)m_date.c_str(), (char*)file_path.c_str(), message, 
                        &nb_hor, sigma_2_h, &nb_ver, sigma_2_v);

}

void DataDump::add_022_quads() {
    auto current_quads = QUADS.at(m_beam_line);

    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            if (k >= m_quad_fetch->get_num_valid_measurements())
                *m_022_file << 0.0;
            else
                *m_022_file << m_quad_fetch->get_quad(current_quads[k])->field * 1000;
        }
        k++;
    }
    *m_022_file << std::endl;
}

void DataDump::add_022_sigmah() {

}

void DataDump::add_022_sigmav() {

}
