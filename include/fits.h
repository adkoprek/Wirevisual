//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Holds the file names for the fit functions
//
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <map>
#include <string>


enum FITS {
    TWO_SIGMA,
    TWO_SIGMA_RED,
    TWO_SIGMA_FIT,
    FWHM,
    FWHM_FIT
};

const std::map<FITS, std::string> FIT_NAMES = {
    {FITS::TWO_SIGMA,       "2Sigma"},
    {FITS::TWO_SIGMA_RED,   "2SigmaRed"},
    {FITS::TWO_SIGMA_FIT,   "2SigmaFit"},
    {FITS::FWHM,            "fwhm"},
    {FITS::FWHM_FIT,        "fwhmFit"}
};
