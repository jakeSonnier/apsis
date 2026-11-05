#include "orbitPropagator.H"

class keplerianPropagationData_t : public propagationData_t
{
        /// \todo Pre-compute some data to propagation data class. Going to just wrap the
        /// satellite object into this to keep the inheritance in line as the SGP4 propagation
        /// model is setup. The keplerian model can be updated for better efficiency later but
        /// is not a priority for now.
public:
        bool initialize (const satellite_t &satellite_);

        const satellite_t &satellite () const;

private:
        std::optional<satellite_t> m_satellite;
};

class keplerianPropagator_t final : public orbitPropagator_t
{
public:
        keplerianPropagator_t ();

        bool initialize (
                const satellite_t &satellite_,
                std::unique_ptr<const propagationData_t> &propagationData_) const override;

        satelliteState_t propagate (const propagationData_t &propagationData_,
                                    timePoint time_) const override;

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

        std::unique_ptr<const propagationData_t> m_keplerianPropagationData;
};