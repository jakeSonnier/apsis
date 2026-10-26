#include "tleParser.H"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tleParser {

    std::vector<satellite_t> loadSatellites(
        const std::string& filename)
    {
        std::ifstream file(filename);

        if (!file) {
            throw std::runtime_error(
                "Could not open TLE file: " + filename
            );
        }

        std::vector<satellite_t> satellites;

        std::string line1;
        std::string line2;

        while (std::getline(file, line1) &&
               std::getline(file, line2)) {

            // TODO: Extract orbital parameters
            // from line1 and line2.

            // TODO: Construct a Satellite object
            // using the extracted parameters.

            // satellites.push_back(satellite);
        }

        return satellites;
    }
}