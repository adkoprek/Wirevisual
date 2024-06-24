#pragma once
#include "cafe.h"
#include "quad_point.h"
#include <string>
#include <vector>


typedef std::vector<double> data_vector;

class QuadFetch {
public:
    QuadFetch();
    ~QuadFetch();
    void fetch(std::vector<std::string> names);
    QuadPoint* get_quad(std::string name);
    int get_num_valid_measurements();

private:
    CAFE* m_cafe;
    int m_num_valid_measurements;
    std::string m_current_quad;
    std::map<std::string, QuadPoint*> m_quads;

    int get_acs();
    int get_field();
    int get_current();
};
