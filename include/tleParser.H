
#pragma once

#include <string>
#include <vector>
#include "satellite.H"

namespace tleParser {
    std::vector<satellite_t> loadSatellites(
        const std::string& filename
    );
}