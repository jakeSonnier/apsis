#include "orbitPropagator.H"
#include "satellite.H"

#pragma once

class sgp4PropagationData_t final : public propagationData_t
{
        friend class sgp4Propagator_t;

public:
        bool initialize (const satellite_t &satellite_);

        std::string toString ();

private:
        double calculateUnkozaiMeanMotion ();
        bool calculateDragCoefficients ();
        bool calculateSecularCoefficients ();
        bool calculateLongPeriodCoefficients ();

        // TLE-derived elements (internal units)
        timePoint m_epoch;
        double m_eccentricity;
        double m_inclination;
        double m_rightAscensionOfAscendingNode;
        double m_argumentOfPerigee;
        double m_meanAnomaly;
        double m_kozaiMeanMotion;
        double m_bstar;

        // Corrected orbital parameters
        double m_unkozaiMeanMotion;
        double m_semiMajorAxis;
        double m_semiLatusRectum;
        double m_perigeeRadius;

        // Drag coefficients
        double m_dragCoefficient1;
        double m_dragCoefficient2;
        double m_dragCoefficient3;
        double m_dragCoefficient4;
        double m_dragCoefficient5;
        double m_eccentricityDragCoefficient1;
        double m_eccentricityDragCoefficient2;
        double m_eccentricityDragCoefficient3;
        double m_meanMotionDragCoefficient1;
        double m_meanMotionDragCoefficient2;
        double m_meanMotionDragCoefficient3;
        double m_meanMotionDragCoefficient4;

        // Additional drag correction coefficients
        double m_argumentOfPerigeeDragCoefficient;
        double m_meanAnomalyDragCoefficient;
        double m_rightAscensionDragCoefficient;

        // Secular coefficients
        double m_meanAnomalyRate;
        double m_argumentOfPerigeeRate;
        double m_rightAscensionRate;

        // Long period coefficients
        double m_longPeriodCoefficientX;
        double m_longPeriodCoefficientY;

        // Propagation mode
        bool m_isDeepSpace;
        bool m_isSimplified;

        // Additional near-Earth propagation parameters
        double m_eta;
        double m_delmo;
        double m_sineOfMeanAnomaly;
        double m_inclinationCorrection;

        // Successful initialization
        bool m_isInitialized = false;
};

class sgp4Propagator_t final : public orbitPropagator_t
{
public:
        sgp4Propagator_t ();

        bool initialize (
                const satellite_t &satellite_,
                std::unique_ptr<const propagationData_t> &propagationData_) const override;

        satelliteState_t propagate (const propagationData_t &propagationData_,
                                    timePoint time_) const override;

private:
        double solveKeplersEquation (double initialEccentricLongitude_,
                                     double eccentricityComponentX_,
                                     double eccentricityComponentY_) const;
};