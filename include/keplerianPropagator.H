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
};