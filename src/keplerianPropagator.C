#include "keplerianPropagator.H"

#include <cmath>
#include <iostream>

keplerianPropagator_t::keplerianPropagator_t ()
{
}

satelliteState_t keplerianPropagator_t::propagate (const satellite_t &satellite_,
                                                   timePoint time_) const
{
        satelliteState_t satelliteState;

        const double deltaTime = elapsedSecondsSinceEpoch (satellite_, time_);
        const double meanMotionRadPerSecond =
                meanMotionRadiansPerSecond (satellite_.orbit ().meanMotion ());
        const double semiMajorAxisKm = semiMajorAxis (meanMotionRadPerSecond);
        const double meanAnomalyRad =
                meanAnomalyInRadians (satellite_.orbit ().meanAnomaly ());
        const double meanAnomalyAtCurrentTime =
                meanAnomalyAtTime (meanAnomalyRad, meanMotionRadPerSecond, deltaTime);
        const double eccentricAnomalyAtCurrentTime = eccentricAnomalyFromMeanAnomaly (
                meanAnomalyAtCurrentTime, satellite_.orbit ().eccentricity ());
        const orbitalPlanePosition_t position =
                orbitalPlanePosition (semiMajorAxisKm,
                                      satellite_.orbit ().eccentricity (),
                                      eccentricAnomalyAtCurrentTime);

        std::cout << "mean motion in radians per second: " << meanMotionRadPerSecond
                  << std::endl;
        std::cout << "semi major axis in km: " << semiMajorAxisKm << std::endl;
        std::cout << "mean anomaly in radians: " << meanAnomalyRad << std::endl;
        std::cout << std::setprecision (12)
                  << "mean anomaly at current time: " << meanAnomalyAtCurrentTime << std::endl;
        std::cout << std::setprecision (12)
                  << "eccentric anomaly at current time: " << eccentricAnomalyAtCurrentTime
                  << std::endl;
        std::cout << "Orbital Position X: " << position.xKm << " Y: " << position.yKm
                  << std::endl;

        std::cout << std::endl;

        return satelliteState;
}

double keplerianPropagator_t::elapsedSecondsSinceEpoch (const satellite_t &satellite_,
                                                        timePoint time_) const
{
        const timePoint epoch = satellite_.orbit ().epoch ();

        return std::chrono::duration<double> (time_ - epoch).count ();
}

double keplerianPropagator_t::meanMotionRadiansPerSecond (const double meanMotion_) const
{
        return meanMotion_ * TWO_PI / SECONDS_PER_DAY;
}

double keplerianPropagator_t::semiMajorAxis (const double meanMotionRadiansPerSecond_) const
{
        return cbrt (EARTH_MU_KM3_PER_SECOND2 / std::pow (meanMotionRadiansPerSecond_, 2));
}

double keplerianPropagator_t::meanAnomalyInRadians (const double meanAnomaly_) const
{
        return meanAnomaly_ * DEGREES_TO_RADIANS;
}

double keplerianPropagator_t::meanAnomalyAtTime (const double meanAnomalyInRadians_,
                                                 const double meanMotionRadiansPerSecond_,
                                                 const double elapsedSecondsSinceEpoch_) const
{
        double meanAnomalyAtTime = meanAnomalyInRadians_
                                   + meanMotionRadiansPerSecond_ * elapsedSecondsSinceEpoch_;

        meanAnomalyAtTime = std::fmod (meanAnomalyAtTime, TWO_PI);

        if (meanAnomalyAtTime < 0.0)
        {
                meanAnomalyAtTime += TWO_PI;
        }

        return meanAnomalyAtTime;
}

double keplerianPropagator_t::eccentricAnomalyFromMeanAnomaly (
        const double meanAnomalyInRadians_, const double eccentricity_) const
{
        double eccentricAnomaly = meanAnomalyInRadians_;

        for (int i = 0; i < MAX_KEPLER_ITERATIONS; i++)
        {
                const double functionValue = eccentricAnomaly
                                             - eccentricity_ * std::sin (eccentricAnomaly)
                                             - meanAnomalyInRadians_;

                const double derivative = 1.0 - eccentricity_ * std::cos (eccentricAnomaly);

                const double correction = functionValue / derivative;

                eccentricAnomaly -= correction;

                if (std::abs (correction) < KEPLER_TOLERANCE)
                {
                        return eccentricAnomaly;
                }
        }

        // Return the last estimate if the iteration limit is reached.
        return eccentricAnomaly;
}

orbitalPlanePosition_t keplerianPropagator_t::orbitalPlanePosition (
        const double semiMajorAxisKm_,
        const double eccentricity_,
        const double eccentricAnomalyRadians_) const
{
        orbitalPlanePosition_t position;

        position.xKm =
                semiMajorAxisKm_ * (std::cos (eccentricAnomalyRadians_) - eccentricity_);
        position.yKm = semiMajorAxisKm_ * std::sqrt (1.0 - std::pow (eccentricity_, 2))
                       * std::sin (eccentricAnomalyRadians_);

        return position;
}