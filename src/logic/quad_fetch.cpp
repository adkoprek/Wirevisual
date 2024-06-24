#include "quad_fetch.h"
#include "epics_commands.h"
#include "quad_point.h"
#include <iostream>
#include <string>
#include <vector>


QuadFetch::QuadFetch() {
    m_cafe = new CAFE();
    m_cafe->channelOpenPolicy.setTimeout(1.0);
}

QuadFetch::~QuadFetch() {
    for (const auto & [key, value] : m_quads)
        delete value;

    m_cafe->closeChannels();
    delete m_cafe;
}

void QuadFetch::fetch(std::vector<std::string> names) {
    m_num_valid_measurements = 0;
    for (const auto & [key, value] : m_quads)
        delete value;

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

        m_num_valid_measurements++;
        quad->valid_data = true;
    }
}

QuadPoint* QuadFetch::get_quad(std::string name) {
    return m_quads.at(name);
}

int QuadFetch::get_num_valid_measurements() {
    return m_num_valid_measurements;
}

int QuadFetch::get_acs() {
    signed int acs;
    std::string pv = m_current_quad + QUADS_PV_ACS;
    int status = m_cafe->get(pv.c_str(), acs);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->acs = acs;
    return 0;
}

int QuadFetch::get_field() {
    double field;
    std::string pv = m_current_quad + QUADS_PV_FIELD;
    int status = m_cafe->get(pv.c_str(), field);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->field = field;
    return 0;
}

int QuadFetch::get_current() {
    double current;
    std::string pv = m_current_quad + QUADS_PV_AMPS;
    int status = m_cafe->get(pv.c_str(), current);
    if (status != ICAFE_NORMAL)
        return -1;

    m_quads.at(m_current_quad)->current = current;
    return 0;
}
