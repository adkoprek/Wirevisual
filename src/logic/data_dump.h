//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Class to dump the measured tada from DataFetch into files
//
// The files that are generated are to be used by other programs
// like Transport and MinT. The files are generated in the 
// directory stored int the env variable $TRANSMESS which should 
// be at default /hipa/op/data/TransMess
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuvernik

#pragma once
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#include "cafe.h"
#include "data_fetch.h"
#include "fits.h"
#include "quad_fetch.h"

// Define some shortenings for types
typedef std::vector<std::string> str_array;
typedef char str9[9];
typedef char str50[50];


class DataDump {
public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    // @param pointer to DataFetch instance with the measured profiles
    DataDump(DataFetch* data_fetch);

    // Deconstructor
    ~DataDump();

    // Dump file with data from profile measurements
    // @param a string vector with the names of every measured beamlines
    // @param a fit from the FITS enum 
    void dump(str_array beam_lines, FITS fit);
    void dump_quads(str_array beam_lines, FITS fit);

    // Get the last file name without the extension
    // @return laste date for file name
    std::string get_last_date();
    
    // Get the last human readeble data that is used in the file headers
    // @return last human readeble date
    std::string get_last_human_date();
    
private:
    /************************************************************
    *                       fields
    ************************************************************/

    CAFE* m_cafe;                       // CAFE instance to get data from epics
    DataFetch* m_data_fetch;            // DataFetch instance pointer, set by constructor
    QuadFetch* m_quad_fetch;            // QuadFetch instance pointer, set by constructor
                                        
    FITS m_fit;                         // Current selected fit, changed by dump()
    std::string m_beam_line;            // Holds the current beamline
    struct std::tm* m_tm;               // Pointer for struct of current time
    std::string m_date;                 // Holds last file name
    std::string m_human_date;           // Holds the date of the last file in human readable form
    str_array m_current_profile_names;  // Holds valid measured profile monitor names of beamline
                                        
    double m_current;                   // Holds the current of the beamline
    std::string m_current_monitor;      // Holds the name of the current monitor of the beamline
    std::string m_current_unit;         // Holds the unit of the current monitor of hte beamline
                                        
    std::ofstream* m_mes_file;          // Pointer for the .mes file
    std::ofstream* m_022_file;          // Pointer for the .022 file
                                        
    std::vector<float> m_sigma2_h;      // Holds current horizontal 2sigma values
    std::vector<float> m_sigma2_v;      // Holds current vertical 2sigma values

    /************************************************************
    *                       functions
    ************************************************************/

    // Function that fetches the current time and stores it in m_tm
    void get_local_time();

    // Generate the file name from the fetched date, the filename structure
    // <beamline>/<beamline>_<year><month><day>_<hour><minute><second>
    // @return the file path with file name without extension
    std::string get_file_path();

    // Get all the points of the current beamline that have valid data
    // and add them to a field
    void fetch_current_profile_names();

    // Fetch the value and unit of the current monitor of the current beamline
    // @return error code
    int fetch_current();

    // Add the actual profile measurement data with some data about 
    // every valid profile to the .mes file
    void add_mes_header();

    // Add the actual profile measurement data with some data about 
    // every valid profile to the .mes file
    // @param the name of the profile
    void add_mes_profile_data(std::string profile);

    // Add the vector data for the x, y or fit values of a profile
    // @param vector with the points to add
    void add_mes_vector(std::vector<double> data);

    // Add the data of the current quadrupole magnets to the .mes file
    void add_mes_quads();

    // Call the external function to cretae the .001 transport file
    void create_transport_file();

    // Add the data of the quadrupole magnets to the .022 file
    void add_022_quads();

    // Add the computed 2sigma values from the earlier computet
    // updateTransportFile function
    // @param vector with the points to add
    void add_022_sigma(std::vector<float> data);

    // Move local .dat file gnerated by mint-snap to the right location
    // mint-snap is an external program that is responsible for dumping 
    // some data from HIPA to a .dat file
    // The target file name isl
    // <beamline>/<beamline>_<date computed above>.dat
    void move_dat_file();

    // Run the external programs mint-snap or mint-run, where just one argument differs
    // @param mint-snap or mint-run
    // @param 0 or the date of the last file
    void mint(std::string program, std::string time_stamp);
};
