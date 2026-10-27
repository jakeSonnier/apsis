#include "satellite.H"
#include "tleParser.H"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

int main ()
{
        std::vector<satellite_t> satellitesLoaded = tleParser::loadSatellites ("tles.csv");

        std::cout << std::setprecision (10);

        for (auto it = satellitesLoaded.begin (); it != satellitesLoaded.end (); it++)
        {
                std::cout << it->toString () << std::endl;
        }

        return 0;
}