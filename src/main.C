#include "coordinateTransform.H"
#include "groundStation.H"
#include "keplerianPropagator.H"
#include "passPrediction.H"
#include "satellite.H"
#include "sgp4Propagator.H"
#include "states.H"
#include "tleParser.H"
#include "visibility.H"

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
        std::cout << "Initializing test ground station in Lafayette, LA" << std::endl;
        double latitude = 30.216667;
        double longitude = -92.033333;
        double altitude = 5.0;
        double minimumElevation = 10.0;
        groundStation_t testGroundStation (latitude, longitude, altitude, minimumElevation);
        std::cout << "Latitude: " << testGroundStation.geographicPosition ().latitude
                  << std::endl;
        std::cout << "Longitude: " << testGroundStation.geographicPosition ().longitude
                  << std::endl;
        std::cout << "Altitude: " << testGroundStation.geographicPosition ().altitude
                  << std::endl;
        std::cout << "Minimum Elevation: " << testGroundStation.minimumElevation ()
                  << std::endl;
        std::cout << "ECEF X: " << testGroundStation.ecefPosition ().coordinates.x
                  << std::endl;
        std::cout << "ECEF Y: " << testGroundStation.ecefPosition ().coordinates.y
                  << std::endl;
        std::cout << "ECEF Z: " << testGroundStation.ecefPosition ().coordinates.z
                  << std::endl;

        std::cout << std::endl;

        std::vector<satellite_t> satellitesLoaded = tleParser::loadSatellites ("tles.csv");

        keplerianPropagator_t keplerianPropagator;
        sgp4Propagator_t sgp4Propagator;

        keplerianPropagationData_t keplerianPropagationData;
        keplerianPropagationData.initialize (satellitesLoaded[0]);

        sgp4PropagationData_t sgp4PropagationData;
        sgp4PropagationData.initialize (satellitesLoaded[0]);

        int searchHours = 48;

        std::vector<passPrediction::validSatellitePass_t> validPasses =
                passPrediction::calculateValidPasses (
                        testGroundStation, sgp4PropagationData, searchHours);

        std::cout << "Found " << validPasses.size () << " passes in the next " << searchHours
                  << " hours" << std::endl
                  << std::endl;

        for (auto pass : validPasses)
        {
                std::cout << "---------------------------" << std::endl;
                std::cout << "AOS: " << pass.acquisitionOfSignal << std::endl;
                std::cout << "LOS: " << pass.lossOfSignal << std::endl;
                std::cout << "Max Elevation: " << pass.maximumElevation << " degrees"
                          << std::endl;
                std::cout << "Total Pass Time: " << pass.totalPassTime << " seconds"
                          << std::endl;
                std::cout << "---------------------------" << std::endl;
                std::cout << std::endl;
        }

        /*while (true)
        {
                const timePoint currentTime = std::chrono::system_clock::now ();

                const satelliteState_t keplerianSatelliteState =
                        keplerianPropagator.propagate (keplerianPropagationData,
        currentTime);

                const satelliteState_t sgp4SatelliteState =
                        sgp4Propagator.propagate (sgp4PropagationData, currentTime);

                const geographicPosition_t keplerianGeographicPosition =
                        coordinateTransform::ecefToGeographic (
                                keplerianSatelliteState.position);

                const geographicPosition_t sgp4GeographicPosition =
                        coordinateTransform::ecefToGeographic
        (sgp4SatelliteState.position);

                const double keplerianLatitudeDegrees =
                        keplerianGeographicPosition.latitude * (180.0 /
        std::numbers::pi);

                const double keplerianLongitudeDegrees =
                        keplerianGeographicPosition.longitude * (180.0 /
        std::numbers::pi);

                const double sgp4LatitudeDegrees =
                        sgp4GeographicPosition.latitude * (180.0 / std::numbers::pi);

                const double sgp4LongitudeDegrees =
                        sgp4GeographicPosition.longitude * (180.0 / std::numbers::pi);

                visibilityResult_t visibilityResult =
                        calculateVisibility (sgp4SatelliteState.position,
        testGroundStation);

                std::cout << "Current time is: " << currentTime << std::endl;

                std::cout << std::endl;

                std::cout << "-- Keplerian Propagation --" << std::endl;
                std::cout << "ISS Current Position: \nLatitude: " <<
        keplerianLatitudeDegrees
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
                          << "\nVelocity Z: " << sgp4SatelliteState.velocity.z <<
        std::endl; std::cout << "---------------------------" << std::endl;

                std::cout << "ISS is currently " << toString (visibilityResult);
                std::cout << std::endl << std::endl;

                std::cout << "Elevation: " << visibilityResult.elevation << std::endl;
                std::cout << "Azimuth: " << visibilityResult.azimuth << std::endl;
                std::cout << "Range: " << visibilityResult.range << std::endl;

                std::this_thread::sleep_for (1s);
        }*/
        return 0;
}