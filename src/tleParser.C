#include "tleParser.H"

#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace tleParser {

    std::vector<satellite_t> loadSatellites(
        const std::string& filename_)
    {
        std::fstream file(filename_);

        if (!file) {
            throw std::runtime_error(
                "Could not open TLE file: " + filename_
            );
        }

        std::vector<satellite_t> satellites;

        std::string name;
        std::string line1;
        std::string line2;

        while (readTLE (file, name, line1, line2))
        {
            satellites.push_back (parseTLE (name, line1, line2));
        }

        std::cout << "Successfully parsed " << satellites.size () 
        << " satellite(s) from TLE data\n" << std::endl;

        return satellites;
    }

    bool readTLE(std::fstream& input_,
         std::string& name_,
         std::string& line1_,
         std::string& line2_)
    {
        if (!std::getline(input_, name_))
        {
            return false; // No next TLE
        }
    
        if (!std::getline(input_, line1_) ||
            !std::getline(input_, line2_))
        {
            throw std::runtime_error("Incomplete TLE record");
        }
    
        return true;
    }

    satellite_t parseTLE (
             std::string& name_,
             std::string& line1_,
             std::string& line2_)
    {
        // std::cout << "parsing TLE!" << std::endl;

        double argumentOfPerigee;
        double bstar;
        double eccentricity;
        int elementNumber;
        char ephemerisType;
        int epochDay;
        int epochYear;
        double inclination;
        double meanAnomaly;
        double meanMotion;
        int revolutionNumber;
        double rightAscensionOfAscendingNode;

        char classification;
        std::string internationalDesignator;
        int number;
    
        // Extract fields from line 1
        number = std::stoi(line1_.substr(2, 5));
        classification = line1_[7];
        internationalDesignator = line1_.substr(9, 8);

        epochYear = std::stoi(line1_.substr(18, 2));
        epochDay = std::stod(line1_.substr(20, 12));

        bstar = parseBstar(line1_.substr(53, 8));

        ephemerisType = line1_[62];

        elementNumber = std::stoi(line1_.substr(64, 4));

        // Extract fields from line 2
        inclination = std::stod(line2_.substr(8, 8));
        rightAscensionOfAscendingNode = std::stod(line2_.substr(17, 8));

        // Eccentricity has an implied decimal point
        std::string eccentricityText =
            "0." + line2_.substr(26, 7);
        eccentricity = std::stod(eccentricityText);

        argumentOfPerigee =
            std::stod(line2_.substr(34, 8));

        meanAnomaly =
            std::stod(line2_.substr(43, 8));

        meanMotion =
            std::stod(line2_.substr(52, 11));

        revolutionNumber = std::stoi(line2_.substr(63, 5));

        orbit_t orbit (argumentOfPerigee,
                       bstar,
                       eccentricity,
                       elementNumber,
                       ephemerisType,
                       epochDay,
                       epochYear,
                       inclination,
                       meanAnomaly,
                       meanMotion,
                       revolutionNumber,
                       rightAscensionOfAscendingNode);

        satellite_t satellite (classification,
                               internationalDesignator,
                               name_,
                               number,
                               orbit);
        
        return satellite;
    }

    double parseBstar(const std::string& field)
    {
        // Expected format: " 12345-3" or "-12345-3"
        // First 6 characters: signed mantissa with implied decimal
        // Last 2 characters: signed exponent

        double mantissa = std::stod(field.substr(0, 6)) * 1e-5;
        int exponent = std::stoi(field.substr(6, 2));

        return mantissa * std::pow(10.0, exponent);
    }
}