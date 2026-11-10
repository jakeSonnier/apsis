#include "constants.H"
#include "visibility.H"

#include <cmath>
#include <iostream>

std::string toString (visibilityResult_t result_)
{
        return result_.visible ? "visible" : "not visible";
}

visibilityResult_t calculateVisibility (const ecefPosition_t &satellitePosition_,
                                        const groundStation_t &groundStation_)
{
        const double satelliteDeltaX = satellitePosition_.coordinates.x
                                       - groundStation_.ecefPosition ().coordinates.x;
        const double satelliteDeltaY = satellitePosition_.coordinates.y
                                       - groundStation_.ecefPosition ().coordinates.y;
        const double satelliteDeltaZ = satellitePosition_.coordinates.z
                                       - groundStation_.ecefPosition ().coordinates.z;

        const double stationLatitudeRadians =
                groundStation_.geographicPosition ().latitude * constants::DEGREES_TO_RADIANS;
        const double stationLongitudeRadians =
                groundStation_.geographicPosition ().longitude * constants::DEGREES_TO_RADIANS;

        const double east = -std::sin (stationLongitudeRadians) * satelliteDeltaX
                            + std::cos (stationLongitudeRadians) * satelliteDeltaY;

        const double north = -std::sin (stationLatitudeRadians)
                                     * std::cos (stationLongitudeRadians) * satelliteDeltaX
                             - std::sin (stationLatitudeRadians)
                                       * std::sin (stationLongitudeRadians) * satelliteDeltaY
                             + std::cos (stationLatitudeRadians) * satelliteDeltaZ;

        const double up = std::cos (stationLatitudeRadians)
                                  * std::cos (stationLongitudeRadians) * satelliteDeltaX
                          + std::cos (stationLatitudeRadians)
                                    * std::sin (stationLongitudeRadians) * satelliteDeltaY
                          + std::sin (stationLatitudeRadians) * satelliteDeltaZ;

        const double range = std::sqrt (east * east + north * north + up * up);

        const double elevation = std::atan2 (up, sqrt (east * east + north * north));

        double azimuth = std::atan2 (east, north);

        if (azimuth < 0.0)
        {
                azimuth += 2.0 * M_PI;
        }

        const double minimumElevation =
                groundStation_.minimumElevation () * constants::DEGREES_TO_RADIANS;

        const bool visible = elevation >= minimumElevation;

        return {visible,
                elevation * constants::RADIANS_TO_DEGREES,
                azimuth * constants::RADIANS_TO_DEGREES,
                range};
}