#include "orbitPropagator.H"

class keplerianPropagator_t final : public orbitPropagator_t
{
public:
        keplerianPropagator_t ();

        satelliteState_t propagate (const satellite_t &satellite,
                                    timePoint time) const override;

private:
        double elapsedSecondsSinceEpoch (const satellite_t &satellite_, timePoint time_) const;

        double meanMotionRadiansPerSecond (const double meanMotion_) const;

        double semiMajorAxis (const double meanMotionRadiansPerSecond_) const;

        double meanAnomalyInRadians (const double meanAnomaly_) const;

        double meanAnomalyAtTime (const double meanAnomalyInRadians_,
                                  const double meanMotionRadiansPerSecond_,
                                  const double elapsedSecondsSinceEpoch_) const;

        double eccentricAnomalyFromMeanAnomaly (const double meanAnomalyInRadians_,
                                                const double eccentricity_) const;

        orbitalPlanePosition_t orbitalPlanePosition (
                const double semiMajorAxisKm_,
                const double eccentricity_,
                const double eccentricAnomalyRadians_) const;

        // Physical constants
        static constexpr double EARTH_MU_KM3_PER_SECOND2 = 398600.4418;
        static constexpr double EARTH_ROTATION_RATE_RAD_PER_SECOND = 7.2921150e-5;

        // Time and angle conversions
        static constexpr double SECONDS_PER_DAY = 86400.0;
        static constexpr double TWO_PI = 6.28318530717958647692;
        static constexpr double DEGREES_TO_RADIANS = 3.14159265358979323846 / 180.0;

        // Numerical solver settings
        static constexpr double KEPLER_TOLERANCE = 1e-12;
        static constexpr int MAX_KEPLER_ITERATIONS = 20;
};