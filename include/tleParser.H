#include "satellite.H"

#include <string>
#include <vector>

#pragma once

namespace tleParser
{
/// \brief Loads satellite from orbital data in tle file
/// \param filename_ The file to load TLE data from
std::vector<satellite_t> loadSatellites (const std::string &filename_);

/// \brief Reads three lines from an input stream, corresponding to TLE name and lines 1 and 2
/// \param input_ Input stream to read lines from
/// \param name_ First line of the TLE, name of the satellite
/// \param line1_ Next line after the name
/// \param line2_ Last line of the TLE
bool readTLE (std::fstream &input_,
              std::string &name_,
              std::string &line1_,
              std::string &line2_);

/// \brief Parses TLE lines into individual satellite object
/// \param name_ First line of the TLE, name of the satellite
/// \param line1_ Next line after the name
/// \param line2_ Last line of the TLE
satellite_t parseTLE (std::string &name_, std::string &line1_, std::string &line2_);

/// \brief Parses the bstar element in a TLE
/// \param field_ the bstar element to parse out
double parseBstar (const std::string &field_);
}