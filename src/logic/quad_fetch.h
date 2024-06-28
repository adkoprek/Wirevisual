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

#pragma once
#include <string>
#include <vector>

#include "cafe.h"
#include "quad_point.h"

// Define a type shortening
typedef std::vector<double> data_vector;


class QuadFetch {
public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    QuadFetch();

    // Destructor
    ~QuadFetch();

    // Fetch a vector of quads by name
    // @param vecotr of quad names
    void fetch(std::vector<std::string> names);

    // Get a fetched quad by mame
    // @param name of the quad
    // @return pointer to the quad
    QuadPoint* get_quad(std::string name);

    // Get how many quads could be fetched successfully
    // @return the number
    int get_num_valid_measurements();

private:
    /************************************************************
    *                       functions
    ************************************************************/

    // Get the acs of the current quad
    // @return error code
    int get_acs();

    // Get the field strenght of the quad in kGauss
    // @return error code
    int get_field();

    // Get the current threw the quad in Ampere
    // @return error code
    int get_current();

    /************************************************************
    *                       fields
    ************************************************************/
    CAFE* m_cafe;                       // Pointer to the internal cafe instance for EPICS access
    int m_num_valid_measurements;       // Storage variable for how many quads were measured valid
    std::string m_current_quad;         // Holds the name of the current quad 
    std::map<std::string, QuadPoint*> m_quads;  // Holds the pointers to the quads by name
};
