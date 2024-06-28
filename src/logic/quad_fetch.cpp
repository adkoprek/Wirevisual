//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// A class to fetch all informations about a quadrupol magnet
//
// This class uses EPICS a library developed by Jan Chrin
// to comunicate with epics. There isn't any class like DataFetch
// to stop this operatiion because it's fast
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <iostream>
#include <string>
#include <vector>

#include "quad_fetch.h"
#include "epics_commands.h"
#include "quad_point.h"


/************************************************************
*                       public
************************************************************/

// Constructor
QuadFetch::QuadFetch() {
    m_cafe = new CAFE();
    m_cafe->channelOpenPolicy.setTimeout(1.0);  // Set timeout for cafe
}

// Destructor
QuadFetch::~QuadFetch() {
    for (const auto & [key, value] : m_quads)
        delete value;

    m_cafe->closeChannels();
    delete m_cafe;
}

// Fetch a vector of quads by name
void QuadFetch::fetch(std::vector<std::string> names) {
    // Reset the internal data
    m_num_valid_measurements = 0;
    for (const auto & [key, value] : m_quads) {
        delete value;
    }
    m_quads.clear();

    // Fetch every quad in a loop
    for (int i = 0; i < names.size(); i++) {
        m_current_quad = names[i];
        auto quad = new QuadPoint();
        quad->name = m_current_quad;
        quad->valid_data = false;
        m_quads.insert({m_current_quad, quad});

        int code = get_acs();
        if (code != 0) {
            std::cerr << "An error was encountered while fetching the acs of the quad \n";
            std::cerr << m_current_quad << "\n";
            continue;
        }

        code = get_field();
        if (code != 0) {
            std::cerr << "An error was encountered while fetching the field of the quad \n";
            std::cerr << m_current_quad << "\n";
            continue;
        }

        code = get_current();
        if (code != 0) {
            std::cerr << "An error was encountered while fetching the current of the quad \n";
            std::cerr << m_current_quad << "\n";
            continue;
        }

        // Only increase if the every data about the quad was fetched successfully
        m_num_valid_measurements++;
        quad->valid_data = true;
    }
}

// Get a fetched quad by mame
QuadPoint* QuadFetch::get_quad(std::string name) {
    return m_quads.at(name);
}

// Get how many quads could be fetched successfully
int QuadFetch::get_num_valid_measurements() {
    return m_num_valid_measurements;
}

/************************************************************
*                       private
************************************************************/

// Get the acs of the current quad
int QuadFetch::get_acs() {
    signed int acs;

    std::string pv = m_current_quad + QUADS_PV_ACS;
    int status = m_cafe->get(pv.c_str(), acs);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->acs = acs;

    return 0;
}

// Get the field strenght of the quad in kGauss
int QuadFetch::get_field() {
    double field;

    std::string pv = m_current_quad + QUADS_PV_FIELD;
    int status = m_cafe->get(pv.c_str(), field);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->field = field;

    return 0;
}

// Get the current threw the quad in Ampere
int QuadFetch::get_current() {
    double current;

    std::string pv = m_current_quad + QUADS_PV_AMPS;
    int status = m_cafe->get(pv.c_str(), current);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->current = current;

    return 0;
}
