//  _    _ _                _                 _
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Class to dump the measured data from DataFetch into files
//
// The files that are generated are to be used by other programs
// like Transport and MinT. The files are generated in the
// directory stored in the env variable $TRANSMESS which should
// be at default /hipa/op/data/TransMess
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sched.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "data_dump.h"
#include "currents.h"
#include "data_fetch.h"
#include "epics_commands.h"
#include "fits.h"
#include "profiles.h"
#include "quad_fetch.h"
#include "quads.h"
#include <sys/wait.h>


// The path where external executables for HIPA are located
#define BD_PATH "/hipa/bd/bin/"

// The external function developed by Mezger, look in create_transport.cpp
extern int updateTransportFile(char *transLine, int nbDevs, str9 *Quads, int *QuadsSign, float *values,
                        str50 *qerrMsg, int nbProfs, str9 *Profs, str50 *perrMsg, float *sigma2,
                        char *broDev, char *broUnit, float broVal, char *actualTime,
                        char *fileName, char *message,
                        int *nbHor, float *sigma2h, int *nbVer, float *sigma2v);



/************************************************************
*                       public
************************************************************/

DataDump::DataDump(DataFetch* data_fetch) {
    m_cafe = new CAFE;
    m_cafe->channelOpenPolicy.setTimeout(1.0);
    m_quad_fetch = new QuadFetch();
    this->m_data_fetch = data_fetch;
}

DataDump::~DataDump() {
    m_cafe->closeChannels();
    delete m_cafe;
    delete m_quad_fetch;
}

// Dump file with data from profile measurements
//   - .mes
//   - .022
//   - .001
//   - .dat
//   - .mint
void DataDump::dump(std::vector<std::string> beam_lines, FITS fit) {
    m_fit = fit;
    get_local_time();

    for (int i = 0; i < beam_lines.size(); i++) {
        m_beam_line = beam_lines[i];

        // File name consists of the current date and time
        auto file_path = get_file_path();
        m_mes_file = new std::ofstream(file_path + ".mes");
        m_022_file = new std::ofstream(file_path + ".022");

        // Call to external mint-snap program to gernerate .dat file
        mint("mint-snap", "0");

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

        // Create the .011 fie that is used with the external program transport
        create_transport_file();

        // Create .022 file
        *m_022_file << std::fixed << std::setw(10) << std::setprecision(1);
        add_022_quads();
        add_022_sigma(m_sigma2_h);
        add_022_sigma(m_sigma2_v);

        // Create MinT files
        move_dat_file();
        mint("mint-run", m_date);

        m_mes_file->close();
        m_022_file->close();
        delete m_mes_file;
        delete m_022_file;
    }
}

// Function to just dump the current settings of the quadrupol magnets
void DataDump::dump_quads(std::vector<std::string> beam_lines, FITS fit) {
    m_fit = fit;
    get_local_time();

    for (int i = 0; i < beam_lines.size(); i++) {
        m_beam_line = beam_lines[i];
        auto file_path = get_file_path();
        m_mes_file = new std::ofstream(file_path + ".mes");

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
        m_quad_fetch->fetch(QUADS.at(m_beam_line));
        add_mes_quads();

        m_mes_file->close();
        delete m_mes_file;
    }
}

// Get the last file name without the extension
std::string DataDump::get_last_date() {
    return m_date;
}

// Get the last human readable data that is used in the file headers
std::string DataDump::get_last_human_date() {
    return m_human_date;
}


/************************************************************
*                       private
************************************************************/

// Function that fetches the current time and stores it in m_tm
void DataDump::get_local_time() {
    time_t now = time(0);
    m_tm = localtime(&now);
    m_tm->tm_year += 1900;
    m_tm->tm_mon += 1;
}

// Generate the file name from the fetched date, the filename structure
// <beamline>/<beamline>_<year><month><day>_<hour><minute><second>
std::string DataDump::get_file_path() {
    std::string file_name;

    m_date = "";
    m_date += std::to_string(m_tm->tm_year);
    if (m_tm->tm_mon < 10) m_date += "0";
    m_date += std::to_string(m_tm->tm_mon);
    if (m_tm->tm_mday < 10) m_date += "0";
    m_date += std::to_string(m_tm->tm_mday);
    m_date += "_";
    if (m_tm->tm_hour < 10) m_date += "0";
    m_date += std::to_string(m_tm->tm_hour);
    if (m_tm->tm_min < 10) m_date += "0";
    m_date += std::to_string(m_tm->tm_min);
    if (m_tm->tm_sec < 10) m_date += "0";
    m_date += std::to_string(m_tm->tm_sec);

    file_name += m_beam_line + "_" + m_date;
    return std::string(getenv("TRANSMESS")) + "/" + m_beam_line + "/" + file_name;
}

// Get all the points of the current beamline that have valid data
// and add them to a field
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

// Fetch the value and unit of the current monitor of the current beamline
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

// Add a header to the .mes file
void DataDump::add_mes_header() {
    // The human readable data has the structure
    // <day>-<month>-<year> <hour>:<minute>:<second>
    m_human_date = "";
    if (m_tm->tm_mday < 10) m_human_date += "0";
    m_human_date += std::to_string(m_tm->tm_mday) + "-";
    if (m_tm->tm_mon < 10) m_human_date += "0";
    m_human_date += std::to_string(m_tm->tm_mon) + "-";
    m_human_date += std::to_string(m_tm->tm_year);
    m_human_date += " ";
    if (m_tm->tm_hour < 10) m_human_date += "0";
    m_human_date += std::to_string(m_tm->tm_hour) + ":";
    if (m_tm->tm_min < 10) m_human_date += "0";
    m_human_date += std::to_string(m_tm->tm_min) + ":";
    if (m_tm->tm_sec < 10) m_human_date += "0";
    m_human_date += std::to_string(m_tm->tm_sec);

    *m_mes_file << "date       \"" << m_human_date << "\"" << std::endl;
    *m_mes_file << "current    " << m_current_monitor << "   ";
    *m_mes_file << std::setprecision(1) << m_current;
    *m_mes_file << "   " << m_current_unit << std::endl;
    *m_mes_file << "beamline   " << m_beam_line << std::endl;
    *m_mes_file << "impuls     ??????   0.000000   \"\"" << std::endl;
    *m_mes_file << "option     " << FIT_NAMES.at(m_fit) << std::endl;
    *m_mes_file << "nbprofs    " << m_current_profile_names.size() << std::endl;
}

// Add the actual profile measurement data with some data about
// every valid profile to the .mes file
void DataDump::add_mes_profile_data(std::string profile) {
    auto point = m_data_fetch->get_data_point(profile);
    if (!point->valid_data) return;
    if (profile[3] == '0') profile.erase(3, 1);

    *m_mes_file << std::setprecision(3) << "Profil     " << profile;
    *m_mes_file << "   offset "  << point->offset;
    *m_mes_file << "   step "    << point->step;
    *m_mes_file << "   nb "      << point->x.size();
    *m_mes_file << "   mean "    << point->mean;
    *m_mes_file << "  meanFit "  << point->mean_fit;
    *m_mes_file << " 4sigma "    << point->sigma_4;
    *m_mes_file << " 4sigmaRed " << point->sigma_4_red;
    *m_mes_file << " 4sigmaFit " << point->sigma_4_fit;
    *m_mes_file << " fwhm "      << point->fwhm;
    *m_mes_file << " fwhmFit "   << point->mean_fit;
    // Add data about in witch direction the profiles where driven
    *m_mes_file << "   dir "     << point->direction << std::endl;

    add_mes_vector(point->x);
    add_mes_vector(point->y);
    add_mes_vector(point->fit);
}

// Add the vector data for the x, y or fit values of a profile
void DataDump::add_mes_vector(std::vector<double> data) {
    *m_mes_file << "           " << std::setprecision(6);
    for (int i = 0; i < data.size(); i++)
        *m_mes_file << data[i] << " ";
    *m_mes_file << std::endl;
}

// Add the data of the current quadrupole magnets to the .mes file
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

// Call the external function to cretae the .001 transport file
void DataDump::create_transport_file() {
    // convert C++ style data into C styled data
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
        // The beamline pktebhe requires other quad_signs
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

    m_sigma2_h = std::vector<float>(sigma_2_h, sigma_2_h + nb_hor);
    m_sigma2_v = std::vector<float>(sigma_2_v, sigma_2_v + nb_ver);
}

// Add the data of the quadrupole magnets to the .022 file
void DataDump::add_022_quads() {
    auto current_quads = QUADS.at(m_beam_line);

    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            *m_022_file << std::setw(10);
            if (k >= m_quad_fetch->get_num_valid_measurements())
                *m_022_file << 0.0;
            else
                *m_022_file << m_quad_fetch->get_quad(current_quads[k])->field * 1000;
            k++;
        }
        *m_022_file << std::endl;
    }
}

// Add the computed 2sigma values from the earlier computet
// updateTransportFile function
void DataDump::add_022_sigma(std::vector<float> data) {
    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 10; j++) {
            *m_022_file << std::setw(10);
            if (k >= data.size())
                *m_022_file << 0.0;
            else
                *m_022_file << data[k];
            k++;
        }
        *m_022_file << std::endl;
    }
}

// Move local .dat file gnerated by mint-snap to the right location
// mint-snap is an external program that is responsible for dumping
// some data from HIPA to a .dat file
// The target file name isl
// <beamline>/<beamline>_<date computed above>.dat
void DataDump::move_dat_file() {
    std::string temp_dat_file;
    temp_dat_file += std::string(getenv("TRANSMESS")) + "/" + m_beam_line + "/";
    temp_dat_file += m_beam_line + "_0.dat";
    int i = 0;
    while (i < 10) {
        if (access(temp_dat_file.c_str(), F_OK) == 0) {
            std::string command = "mv " + temp_dat_file + " ";
            command += std::string(getenv("TRANSMESS")) + "/"  + m_beam_line + "/" + m_beam_line + "_" + m_date + ".dat";
            char command_str[command.length() + 1];
            std::cout << "Move command: " << command << std::endl;
            strcpy(command_str, command.c_str());
            system(command_str);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        i++;
        if (i == 1) std::cout << "Waiting for snap file ..." << std::endl;
        if (i >= 10) std::cout << "Giving up, snap file not present" << std::endl;
    }
}

// Run the external programs mint-snap or mint-run, where just one argument differs
void DataDump::mint(std::string program, std::string time_stamp) {
    pid_t pid = fork();

    if (pid == 0) {
        char program_str[20], command[128], beam_line[20], time_stamp_str[256];
        strcpy(program_str, program.c_str());
        strcpy(beam_line, m_beam_line.c_str());
        strcpy(command, BD_PATH);
        strcpy(command, program_str);
        strcpy(time_stamp_str, time_stamp.c_str());
        char* options[4];
        options[0] = command;
        options[1] = beam_line;
        options[2] = time_stamp_str;
        options[3] = NULL;
        execvp(command, options);

        std::cerr << "An error occured while trying to run \n" << program << "\"";
        std::cerr << ", " << std::strerror(errno);
        exit(1);
    } else if (pid > 0) {} else  {
        std::cerr << "An error occured while trying to fork the process, ";
        std::cerr << "continuing without calling mint";
        return;
    }
}
