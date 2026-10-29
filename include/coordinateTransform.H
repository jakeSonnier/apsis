#include "satellite.H"
#include "satelliteState.H"

#include <chrono>

using timePoint = std::chrono::system_clock::time_point;

namespace coordinateTransform
{
eciPosition_t orbitalPlaneToEci (const orbitalPlanePosition_t &position_,
                                 const orbit_t &orbit_);

ecefPosition_t eciToEcef (const eciPosition_t &position_, timePoint time_);

geographicPosition_t ecefToGeographic (const ecefPosition_t &position_);

ecefPosition_t geographicToEcef (const geographicPosition_t &position_);

double greenwichMeanSiderealTime (const timePoint time_);
};