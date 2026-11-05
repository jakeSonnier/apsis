#include "coordinateTransform.H"
#include "keplerianPropagator.H"
#include "satellite.H"
#include "satelliteState.H"
#include "sgp4Propagator.H"
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
        sgp4Propagator_t sgp4Propagator;

        keplerianPropagationData_t keplerianPropagationData;
        keplerianPropagationData.initialize (satellitesLoaded[0]);

        sgp4PropagationData_t sgp4PropagationData;
        sgp4PropagationData.initialize (satellitesLoaded[0]);

        while (true)
        {
                const timePoint currentTime = std::chrono::system_clock::now ();

                const satelliteState_t keplerianSatelliteState =
                        keplerianPropagator.propagate (keplerianPropagationData, currentTime);

                const satelliteState_t sgp4SatelliteState =
                        sgp4Propagator.propagate (sgp4PropagationData, currentTime);

                const geographicPosition_t keplerianGeographicPosition =
                        coordinateTransform::ecefToGeographic (
                                keplerianSatelliteState.position);

                const double keplerianLatitudeDegrees =
                        keplerianGeographicPosition.latitude * (180.0 / std::numbers::pi);

                const double keplerianLongitudeDegrees =
                        keplerianGeographicPosition.longitude * (180.0 / std::numbers::pi);

                std::cout << "Keplerian Propagation ->" << std::endl;
                std::cout << "ISS Current Position: \nLatitude: " << keplerianLatitudeDegrees
                          << "\nLongitude: " << keplerianLongitudeDegrees
                          << "\nAltitude: " << keplerianGeographicPosition.altitude
                          << "\nVelocity: " << keplerianSatelliteState.velocity.x << "\n"
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