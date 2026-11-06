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
        std::cout << sgp4PropagationData.toString ();

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

                const geographicPosition_t sgp4GeographicPosition =
                        coordinateTransform::ecefToGeographic (sgp4SatelliteState.position);

                const double keplerianLatitudeDegrees =
                        keplerianGeographicPosition.latitude * (180.0 / std::numbers::pi);

                const double keplerianLongitudeDegrees =
                        keplerianGeographicPosition.longitude * (180.0 / std::numbers::pi);

                const double sgp4LatitudeDegrees =
                        sgp4GeographicPosition.latitude * (180.0 / std::numbers::pi);

                const double sgp4LongitudeDegrees =
                        sgp4GeographicPosition.longitude * (180.0 / std::numbers::pi);

                std::cout << std::endl;

                std::cout << "-- Keplerian Propagation --" << std::endl;
                std::cout << "ISS Current Position: \nLatitude: " << keplerianLatitudeDegrees
                          << "\nLongitude: " << keplerianLongitudeDegrees
                          << "\nAltitude: " << keplerianGeographicPosition.altitude
                          << "\nVelocity X: " << keplerianSatelliteState.velocity.x
                          << "\nVelocity Y: " << keplerianSatelliteState.velocity.y
                          << "\nVelocity Z: " << keplerianSatelliteState.velocity.z
                          << std::endl;
                std::cout << "---------------------------" << std::endl;

                std::cout << "-- SGP4 Propagation --" << std::endl;
                std::cout << "ISS Current Position: \nLatitude: " << sgp4LatitudeDegrees
                          << "\nLongitude: " << sgp4LongitudeDegrees
                          << "\nAltitude: " << sgp4GeographicPosition.altitude
                          << "\nVelocity X: " << sgp4SatelliteState.velocity.x
                          << "\nVelocity Y: " << sgp4SatelliteState.velocity.y
                          << "\nVelocity Z: " << sgp4SatelliteState.velocity.z << std::endl;
                std::cout << "---------------------------" << std::endl;

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