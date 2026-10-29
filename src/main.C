#include "coordinateTransform.H"
#include "keplerianPropagator.H"
#include "satellite.H"
#include "satelliteState.H"
#include "tleParser.H"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

using timePoint = std::chrono::system_clock::time_point;
using namespace std::chrono_literals;

int main ()
{
        std::vector<satellite_t> satellitesLoaded = tleParser::loadSatellites ("tles.csv");

        std::cout << std::setprecision (10);

        keplerianPropagator_t keplerianPropagator;

        while (true)
        {
                const timePoint currentTime = std::chrono::system_clock::now ();
                const satelliteState_t satelliteState =
                        keplerianPropagator.propagate (satellitesLoaded[0], currentTime);
                const geographicPosition_t geographicPosition =
                        coordinateTransform::ecefToGeographic (satelliteState.position);

                const double latitudeDegrees =
                        geographicPosition.latitude * (180.0 / std::numbers::pi);

                const double longitudeDegrees =
                        geographicPosition.longitude * (180.0 / std::numbers::pi);

                std::cout << "ISS Current Position: \nLatitude: " << latitudeDegrees
                          << "\nLongitude: " << longitudeDegrees
                          << "\nAltitude: " << geographicPosition.altitude
                          << "\nVelocity: " << satelliteState.velocity.x << "\n"
                          << "\n"
                          << std::endl;

                std::this_thread::sleep_for (1s);
        }

        // for (auto it = satellitesLoaded.begin (); it != satellitesLoaded.end (); it++)
        //{
        //         std::cout << it->orbit ().epoch () << std::endl;
        //
        //        const timePoint currentTime = std::chrono::system_clock::now ();
        //        keplerianPropagator.propagate (*it, currentTime);
        //}

        return 0;
}