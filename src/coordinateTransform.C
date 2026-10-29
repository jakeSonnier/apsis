#include "constants.H"
#include "coordinateTransform.H"

#include <cmath>
#include <iostream>

eciPosition_t coordinateTransform::orbitalPlaneToEci (const orbitalPlanePosition_t &position_,
                                                      const orbit_t &orbit_)
{
        const double raan =
                orbit_.rightAscensionOfAscendingNode () * constants::DEGREES_TO_RADIANS;
        const double inclination = orbit_.inclination () * constants::DEGREES_TO_RADIANS;
        const double argumentOfPerigee =
                orbit_.argumentOfPerigee () * constants::DEGREES_TO_RADIANS;

        const double cosRaan = std::cos (raan);
        const double sinRaan = std::sin (raan);
        const double cosInc = std::cos (inclination);
        const double sinInc = std::sin (inclination);
        const double cosArg = std::cos (argumentOfPerigee);
        const double sinArg = std::sin (argumentOfPerigee);

        eciPosition_t result{};

        result.coordinates.x =
                position_.xKm * (cosRaan * cosArg - sinRaan * sinArg * cosInc)
                - position_.yKm * (cosRaan * sinArg + sinRaan * cosArg * cosInc);

        result.coordinates.y =
                position_.xKm * (sinRaan * cosArg + cosRaan * sinArg * cosInc)
                + position_.yKm * (cosRaan * cosArg * cosInc - sinRaan * sinArg);

        result.coordinates.z =
                position_.xKm * sinArg * sinInc + position_.yKm * cosArg * sinInc;

        return result;
}

ecefPosition_t coordinateTransform::eciToEcef (const eciPosition_t &position_, timePoint time_)
{
        const double theta = greenwichMeanSiderealTime (time_);

        const double cosTheta = std::cos (theta);
        const double sinTheta = std::sin (theta);

        ecefPosition_t result{};

        result.coordinates.x =
                position_.coordinates.x * cosTheta + position_.coordinates.y * sinTheta;

        result.coordinates.y =
                -position_.coordinates.x * sinTheta + position_.coordinates.y * cosTheta;

        result.coordinates.z = position_.coordinates.z;

        return result;
}

geographicPosition_t coordinateTransform::ecefToGeographic (const ecefPosition_t &position_)
{
        constexpr double a = 6378.137; // WGS84 equatorial radius, km
        constexpr double f = 1.0 / 298.257223563;
        constexpr double e2 = f * (2.0 - f);

        const double x = position_.coordinates.x;
        const double y = position_.coordinates.y;
        const double z = position_.coordinates.z;

        const double p = std::hypot (x, y);

        geographicPosition_t result{};

        // Longitude is undefined at the poles.
        const double longitude = std::atan2 (y, x);

        // Handle the poles separately to avoid division by zero.
        if (p < 1e-12)
        {
                result.latitude = std::copysign (std::numbers::pi / 2.0, z);

                result.longitude = 0.0;
                result.altitude = std::abs (z) - a * (1.0 - f);

                return result;
        }

        // Initial latitude estimate.
        double latitude = std::atan2 (z, p * (1.0 - e2));
        double altitude = 0.0;

        // Iteratively refine geodetic latitude.
        for (int i = 0; i < 10; ++i)
        {
                const double sinLat = std::sin (latitude);

                const double N = a / std::sqrt (1.0 - e2 * sinLat * sinLat);

                altitude = p / std::cos (latitude) - N;

                const double nextLatitude =
                        std::atan2 (z, p * (1.0 - e2 * N / (N + altitude)));

                if (std::abs (nextLatitude - latitude) < 1e-12)
                {
                        latitude = nextLatitude;
                        break;
                }

                latitude = nextLatitude;
        }

        // Recompute altitude using the final latitude.
        const double sinLat = std::sin (latitude);
        const double N = a / std::sqrt (1.0 - e2 * sinLat * sinLat);

        altitude = p / std::cos (latitude) - N;

        result.latitude = latitude;
        result.longitude = longitude;
        result.altitude = altitude;

        return result;
}

ecefPosition_t coordinateTransform::geographicToEcef (const geographicPosition_t &position_)
{
        constexpr double a = 6378.137;
        constexpr double f = 1.0 / 298.257223563;
        constexpr double e2 = f * (2.0 - f);

        const double latitude = position_.latitude;
        const double longitude = position_.longitude;
        const double altitude = position_.altitude;

        const double sinLat = std::sin (latitude);
        const double cosLat = std::cos (latitude);
        const double sinLon = std::sin (longitude);
        const double cosLon = std::cos (longitude);

        const double N = a / std::sqrt (1.0 - e2 * sinLat * sinLat);

        ecefPosition_t result{};

        result.coordinates.x = (N + altitude) * cosLat * cosLon;

        result.coordinates.y = (N + altitude) * cosLat * sinLon;

        result.coordinates.z = (N * (1.0 - e2) + altitude) * sinLat;

        return result;
}

double coordinateTransform::greenwichMeanSiderealTime (const timePoint time_)
{
        using namespace std::chrono;

        // Seconds since Unix epoch, not satellite orbital epoch.
        const double secondsSinceUnixEpoch =
                duration<double> (time_ - system_clock::time_point{}).count ();

        const double jd = 2440587.5 + secondsSinceUnixEpoch / 86400.0;

        const double T = (jd - 2451545.0) / 36525.0;

        double gmstDegrees = 280.46061837 + 360.98564736629 * (jd - 2451545.0)
                             + 0.000387933 * T * T - (T * T * T) / 38710000.0;

        gmstDegrees = std::fmod (gmstDegrees, 360.0);

        if (gmstDegrees < 0.0)
                gmstDegrees += 360.0;
        return gmstDegrees * (std::numbers::pi / 180.0);
}