#include "satellite.H"

#include <string>
#include <vector>

#pragma once

namespace tleParser
{
std::vector<satellite_t> loadSatellites (const std::string &filename_);

bool readTLE (std::fstream &input_,
              std::string &name_,
              std::string &line1_,
              std::string &line2_);

satellite_t parseTLE (std::string &name_, std::string &line1_, std::string &line2_);

double parseBstar (const std::string &field);
}