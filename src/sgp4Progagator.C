#include "constants.H"
#include "coordinateTransform.H"
#include "sgp4Propagator.H"
#include "wgs72Constants.H"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

bool sgp4PropagationData_t::initialize (const satellite_t &satellite_)
{
        std::cout << "Starting initialize for SGP4 propagation data" << std::endl;

        // TLE-derived elements (internal units)
        m_epoch = satellite_.orbit ().epoch ();
        m_eccentricity = satellite_.orbit ().eccentricity ();
        m_inclination = satellite_.orbit ().inclination () * constants::DEGREES_TO_RADIANS;
        m_rightAscensionOfAscendingNode = satellite_.orbit ().rightAscensionOfAscendingNode ()
                                          * constants::DEGREES_TO_RADIANS;
        m_argumentOfPerigee =
                satellite_.orbit ().argumentOfPerigee () * constants::DEGREES_TO_RADIANS;
        m_meanAnomaly = satellite_.orbit ().meanAnomaly () * constants::DEGREES_TO_RADIANS;
        m_kozaiMeanMotion = satellite_.orbit ().meanMotion () * constants::TWO_PI
                            / constants::MINUTES_PER_DAY;
        m_bstar = satellite_.orbit ().bstar ();

        if (!std::isfinite (m_eccentricity) || !std::isfinite (m_inclination)
            || !std::isfinite (m_kozaiMeanMotion) || !std::isfinite (m_bstar))
        {
                std::cerr << "Invalid TLE derived values for SGP4 initialization!"
                          << std::endl;
                return false;
        }

        if (m_eccentricity < 0.0 || m_eccentricity >= 1.0 || m_kozaiMeanMotion <= 0.0)
        {
                std::cerr << "Invalid TLE derived values for SGP4 initialization!"
                          << std::endl;
                return false;
        }

        const double eccentricitySquared = m_eccentricity * m_eccentricity;
        const double oneMinusEccentricitySquared = 1.0 - eccentricitySquared;

        // Corrected orbital parameters
        m_unkozaiMeanMotion = calculateUnkozaiMeanMotion ();
        m_semiMajorAxis = std::pow (wgs72::XKE / m_unkozaiMeanMotion, 2.0 / 3.0);
        m_semiLatusRectum = m_semiMajorAxis * oneMinusEccentricitySquared;
        m_perigeeRadius = m_semiMajorAxis * (1.0 - m_eccentricity);

        if (!std::isfinite (m_unkozaiMeanMotion) || !std::isfinite (m_semiMajorAxis)
            || !std::isfinite (m_semiLatusRectum) || !std::isfinite (m_perigeeRadius))
        {
                std::cerr << "Invalid corrected orbital parameters!" << std::endl;
                return false;
        }

        if (m_unkozaiMeanMotion <= 0.0 || m_semiMajorAxis <= 0.0 || m_semiLatusRectum <= 0.0
            || m_perigeeRadius <= 0.0)
        {
                std::cerr << "Non-positive corrected orbital parameters!" << std::endl;
                return false;
        }

        // Simplified drag treatment, call before calculating drag coefficients
        const double perigeeAltitude =
                (m_perigeeRadius - 1.0) * wgs72::EARTH_EQUATORIAL_RADIUS;
        m_isSimplified = perigeeAltitude < 220.0;

        if (!calculateDragCoefficients ())
        {
                return false;
        }

        if (!calculateSecularCoefficients ())
        {
                return false;
        }

        if (!calculateLongPeriodCoefficients ())
        {
                return false;
        }

        // Propagation mode
        const double orbitalPeriod = constants::TWO_PI / m_unkozaiMeanMotion;
        m_isDeepSpace = orbitalPeriod >= 225.0;

        // Additional near-Earth propagation parameters
        const double meanAnomalyCorrection = 1.0 + m_eta * std::cos (m_meanAnomaly);
        m_delmo = meanAnomalyCorrection * meanAnomalyCorrection * meanAnomalyCorrection;

        m_sineOfMeanAnomaly = std::sin (m_meanAnomaly);

        const double cosineInclination = std::cos (m_inclination);
        m_inclinationCorrection = 7.0 * cosineInclination * cosineInclination - 1.0;

        m_isInitialized = true;

        return true;
}

std::string sgp4PropagationData_t::toString ()
{
        std::ostringstream stream;

        stream << std::scientific << std::setprecision (12);

        stream << "SGP4 Propagation Data:" << std::endl;

        // TLE-derived elements
        stream << "\n--- TLE-Derived Elements ---" << std::endl;
        stream << "Epoch: " << m_epoch << std::endl;
        stream << "Eccentricity: " << m_eccentricity << std::endl;
        stream << "Inclination: " << m_inclination << " rad" << std::endl;
        stream << "Right Ascension of Ascending Node: " << m_rightAscensionOfAscendingNode
               << " rad" << std::endl;
        stream << "Argument of Perigee: " << m_argumentOfPerigee << " rad" << std::endl;
        stream << "Mean Anomaly: " << m_meanAnomaly << " rad" << std::endl;
        stream << "Kozai Mean Motion: " << m_kozaiMeanMotion << " rad/min" << std::endl;
        stream << "BSTAR: " << m_bstar << std::endl;

        // Corrected orbital parameters
        stream << "\n--- Corrected Orbital Parameters ---" << std::endl;
        stream << "Un-Kozai Mean Motion: " << m_unkozaiMeanMotion << " rad/min" << std::endl;
        stream << "Semi-Major Axis: " << m_semiMajorAxis << " Earth radii" << std::endl;
        stream << "Semi-Latus Rectum: " << m_semiLatusRectum << " Earth radii" << std::endl;
        stream << "Perigee Radius: " << m_perigeeRadius << " Earth radii" << std::endl;

        // Drag coefficients
        stream << "\n--- Drag Coefficients ---" << std::endl;
        stream << "Drag Coefficient 1 (cc1): " << m_dragCoefficient1 << std::endl;
        stream << "Drag Coefficient 2 (cc2): " << m_dragCoefficient2 << std::endl;
        stream << "Drag Coefficient 3 (cc3): " << m_dragCoefficient3 << std::endl;
        stream << "Drag Coefficient 4 (cc4): " << m_dragCoefficient4 << std::endl;
        stream << "Drag Coefficient 5 (cc5): " << m_dragCoefficient5 << std::endl;

        stream << "Eccentricity Drag Coefficient 1 (d2): " << m_eccentricityDragCoefficient1
               << std::endl;
        stream << "Eccentricity Drag Coefficient 2 (d3): " << m_eccentricityDragCoefficient2
               << std::endl;
        stream << "Eccentricity Drag Coefficient 3 (d4): " << m_eccentricityDragCoefficient3
               << std::endl;

        stream << "Mean Motion Drag Coefficient 1 (t2cof): " << m_meanMotionDragCoefficient1
               << std::endl;
        stream << "Mean Motion Drag Coefficient 2 (t3cof): " << m_meanMotionDragCoefficient2
               << std::endl;
        stream << "Mean Motion Drag Coefficient 3 (t4cof): " << m_meanMotionDragCoefficient3
               << std::endl;
        stream << "Mean Motion Drag Coefficient 4 (t5cof): " << m_meanMotionDragCoefficient4
               << std::endl;

        // Additional drag correction coefficients
        stream << "\n--- Additional Drag Correction Coefficients ---" << std::endl;

        stream << "Argument of Perigee Drag Coefficient (omgcof): "
               << m_argumentOfPerigeeDragCoefficient << std::endl;

        stream << "Mean Anomaly Drag Coefficient (xmcof): " << m_meanAnomalyDragCoefficient
               << std::endl;

        stream << "Right Ascension Drag Coefficient (nodecf): "
               << m_rightAscensionDragCoefficient << std::endl;

        // Secular coefficients
        stream << "\n--- Secular Coefficients ---" << std::endl;
        stream << "Mean Anomaly Rate: " << m_meanAnomalyRate << " rad/min" << std::endl;
        stream << "Argument of Perigee Rate: " << m_argumentOfPerigeeRate << " rad/min"
               << std::endl;
        stream << "Right Ascension Rate: " << m_rightAscensionRate << " rad/min" << std::endl;

        // Long-period coefficients
        stream << "\n--- Long-Period Coefficients ---" << std::endl;
        stream << "Long-Period Coefficient X (xlcof): " << m_longPeriodCoefficientX
               << std::endl;
        stream << "Long-Period Coefficient Y (aycof): " << m_longPeriodCoefficientY
               << std::endl;

        // Propagation mode
        stream << "\n--- Propagation Configuration ---" << std::endl;
        stream << "Deep Space: " << (m_isDeepSpace ? "Yes" : "No") << std::endl;
        stream << "Simplified Drag: " << (m_isSimplified ? "Yes" : "No") << std::endl;

        // Additional near-Earth propagation parameters
        stream << "\n--- Additional Near-Earth Parameters ---" << std::endl;
        stream << "Eta: " << m_eta << std::endl;
        stream << "Delta Mean Anomaly (delmo): " << m_delmo << std::endl;
        stream << "Sine of Mean Anomaly (sinmao): " << m_sineOfMeanAnomaly << std::endl;
        stream << "Inclination Correction (x7thm1): " << m_inclinationCorrection << std::endl;

        // Initialization status
        stream << "\n--- Initialization Status ---" << std::endl;
        stream << "Initialized: " << (m_isInitialized ? "Yes" : "No") << std::endl;

        stream << std::endl;

        return stream.str ();
}

double sgp4PropagationData_t::calculateUnkozaiMeanMotion ()
{
        // Intermediate values
        const double eccentricitySquared = m_eccentricity * m_eccentricity;
        const double oneMinusEccentricitySquared = 1.0 - eccentricitySquared;
        const double eccentricityFactor = std::sqrt (oneMinusEccentricitySquared);
        const double cosineInclination = std::cos (m_inclination);
        const double cosineInclinationSquared = cosineInclination * cosineInclination;
        const double sineInclination = std::sin (m_inclination);
        const double inclinationFactor1 = 3.0 * cosineInclinationSquared - 1.0;
        const double inclinationFactor2 = 1.0 - 5.0 * cosineInclinationSquared;

        const double initialSemiMajorAxis =
                std::pow (wgs72::XKE / m_kozaiMeanMotion, 2.0 / 3.0);

        const double initialMeanMotionCorrection =
                (0.75 * wgs72::SECOND_ZONAL_HARMONIC * inclinationFactor1)
                / (eccentricityFactor * oneMinusEccentricitySquared);

        const double firstMeanMotionCorrection =
                initialMeanMotionCorrection / (initialSemiMajorAxis * initialSemiMajorAxis);

        const double correctedPreliminarySemiMajorAxis =
                initialSemiMajorAxis
                * (1.0 - firstMeanMotionCorrection * firstMeanMotionCorrection
                   - firstMeanMotionCorrection
                             * (1.0 / 3.0
                                + (134.0 / 81.0) * firstMeanMotionCorrection
                                          * firstMeanMotionCorrection));

        const double finalMeanMotionCorrection =
                initialMeanMotionCorrection
                / (correctedPreliminarySemiMajorAxis * correctedPreliminarySemiMajorAxis);

        return m_kozaiMeanMotion / (1.0 + finalMeanMotionCorrection);
}

bool sgp4PropagationData_t::calculateDragCoefficients ()
{
        // Drag coefficients

        const double cosineInclination = std::cos (m_inclination);
        const double sineInclination = std::sin (m_inclination);
        const double cosineInclinationSquared = cosineInclination * cosineInclination;

        const double con41 = 3.0 * cosineInclinationSquared - 1.0;
        const double x1mth2 = 1.0 - cosineInclinationSquared;

        // Atmospheric parameters
        double atmosphericS = 78.0 / wgs72::EARTH_EQUATORIAL_RADIUS + 1.0;

        double atmosphericQoms2t = std::pow (42.0 / wgs72::EARTH_EQUATORIAL_RADIUS, 4.0);

        // Adjust atmospheric parameters for low perigee satellites
        if (m_perigeeRadius
            < 1.0 + wgs72::LOW_PERIGEE_THRESHOLD / wgs72::EARTH_EQUATORIAL_RADIUS)
        {
                atmosphericS = m_perigeeRadius - 1.0 - 78.0 / wgs72::EARTH_EQUATORIAL_RADIUS;

                if (m_perigeeRadius < 1.0 + 98.0 / wgs72::EARTH_EQUATORIAL_RADIUS)
                {
                        atmosphericS = 20.0 / wgs72::EARTH_EQUATORIAL_RADIUS;
                }

                atmosphericQoms2t =
                        std::pow (120.0 / wgs72::EARTH_EQUATORIAL_RADIUS - atmosphericS, 4.0);
        }

        // Intermediate atmospheric drag parameters
        const double tsi = 1.0 / (m_semiMajorAxis - atmosphericS);

        m_eta = m_semiMajorAxis * m_eccentricity * tsi;
        const double etaSquared = m_eta * m_eta;
        const double eccentricityEta = m_eccentricity * m_eta;

        const double psiSquared = std::abs (1.0 - etaSquared);

        const double coef = atmosphericQoms2t * std::pow (tsi, 4.0);
        const double coef1 = coef / std::pow (psiSquared, 3.5);

        // Calculate drag coefficient 2 (cc2)
        m_dragCoefficient2 =
                coef1 * m_unkozaiMeanMotion
                * (m_semiMajorAxis
                           * (1.0 + 1.5 * etaSquared + eccentricityEta * (4.0 + etaSquared))
                   + 0.375 * wgs72::SECOND_ZONAL_HARMONIC * tsi / psiSquared * con41
                             * (8.0 + 3.0 * etaSquared * (8.0 + etaSquared)));

        // Calculate drag coefficient 1 (cc1)
        m_dragCoefficient1 = m_bstar * m_dragCoefficient2;

        // Calculate drag coefficient 3 (cc3)
        if (m_eccentricity > 1.0e-4)
        {
                m_dragCoefficient3 =
                        -2.0 * coef * tsi
                        * (wgs72::THIRD_ZONAL_HARMONIC / wgs72::SECOND_ZONAL_HARMONIC)
                        * m_unkozaiMeanMotion * sineInclination / m_eccentricity;
        }
        else
        {
                m_dragCoefficient3 = 0.0;
        }

        // Calculate drag coefficient 4 (cc4)
        m_dragCoefficient4 =
                2.0 * m_unkozaiMeanMotion * coef1 * m_semiMajorAxis
                * (1.0 - m_eccentricity * m_eccentricity)
                * (m_eta * (2.0 + 0.5 * etaSquared) + m_eccentricity * (0.5 + 2.0 * etaSquared)
                   - wgs72::SECOND_ZONAL_HARMONIC * tsi / (m_semiMajorAxis * psiSquared)
                             * (-3.0 * con41
                                        * (1.0 - 2.0 * eccentricityEta
                                           + etaSquared * (1.5 - 0.5 * eccentricityEta))
                                + 0.75 * x1mth2
                                          * (2.0 * etaSquared
                                             - eccentricityEta * (1.0 + etaSquared))
                                          * std::cos (2.0 * m_argumentOfPerigee)));

        // Calculate drag coefficient 5 (cc5)
        m_dragCoefficient5 =
                2.0 * coef1 * m_semiMajorAxis * (1.0 - m_eccentricity * m_eccentricity)
                * (1.0 + 2.75 * (etaSquared + eccentricityEta) + eccentricityEta * etaSquared);

        // Eccentricity drag coefficients (d2, d3, d4)

        // Mean-motion drag coefficients (t2cof, t3cof, t4cof, t5cof)

        // First-order mean-motion drag coefficient (t2cof)
        m_meanMotionDragCoefficient1 = 1.5 * m_dragCoefficient1;

        // Higher-order coefficients are only required when simplified
        // drag treatment is disabled.
        if (!m_isSimplified)
        {
                // Eccentricity drag coefficient 1 (d2)
                m_eccentricityDragCoefficient1 =
                        4.0 * m_semiMajorAxis * tsi * m_dragCoefficient1 * m_dragCoefficient1;

                const double eccentricityDragIntermediate =
                        m_eccentricityDragCoefficient1 * tsi * m_dragCoefficient1 / 3.0;

                // Eccentricity drag coefficient 2 (d3)
                m_eccentricityDragCoefficient2 =
                        (17.0 * m_semiMajorAxis + atmosphericS) * eccentricityDragIntermediate;

                // Eccentricity drag coefficient 3 (d4)
                m_eccentricityDragCoefficient3 =
                        0.5 * eccentricityDragIntermediate * m_semiMajorAxis * tsi
                        * (221.0 * m_semiMajorAxis + 31.0 * atmosphericS) * m_dragCoefficient1;

                // Mean-motion drag coefficient 2 (t3cof)
                m_meanMotionDragCoefficient2 = m_eccentricityDragCoefficient1
                                               + 2.0 * m_dragCoefficient1 * m_dragCoefficient1;

                // Mean-motion drag coefficient 3 (t4cof)
                m_meanMotionDragCoefficient3 =
                        0.25
                        * (3.0 * m_eccentricityDragCoefficient2
                           + m_dragCoefficient1
                                     * (12.0 * m_eccentricityDragCoefficient1
                                        + 10.0 * m_dragCoefficient1 * m_dragCoefficient1));

                // Mean-motion drag coefficient 4 (t5cof)
                m_meanMotionDragCoefficient4 =
                        0.2
                        * (3.0 * m_eccentricityDragCoefficient3
                           + 12.0 * m_dragCoefficient1 * m_eccentricityDragCoefficient2
                           + 6.0 * m_eccentricityDragCoefficient1
                                     * m_eccentricityDragCoefficient1
                           + 15.0 * m_dragCoefficient1 * m_dragCoefficient1
                                     * m_eccentricityDragCoefficient1
                           + 4.0 * m_dragCoefficient1 * m_dragCoefficient1
                                     * m_dragCoefficient1);
        }
        else
        {
                // Higher-order coefficients are unused with simplified drag.
                m_eccentricityDragCoefficient1 = 0.0;
                m_eccentricityDragCoefficient2 = 0.0;
                m_eccentricityDragCoefficient3 = 0.0;

                m_meanMotionDragCoefficient2 = 0.0;
                m_meanMotionDragCoefficient3 = 0.0;
                m_meanMotionDragCoefficient4 = 0.0;
        }

        // Argument of perigee drag coefficient
        m_argumentOfPerigeeDragCoefficient =
                m_bstar * m_dragCoefficient3 * std::cos (m_argumentOfPerigee);

        // Mean anomaly drag coefficient
        if (m_eccentricity > 1.0e-4)
        {
                const double eccentricityEta = m_eccentricity * m_eta;

                m_meanAnomalyDragCoefficient = -(2.0 / 3.0) * coef * m_bstar / eccentricityEta;
        }
        else
        {
                m_meanAnomalyDragCoefficient = 0.0;
        }

        // Right ascension drag coefficient
        const double betaSquared = 1.0 - m_eccentricity * m_eccentricity;

        const double j2NodalRate = -1.5 * wgs72::SECOND_ZONAL_HARMONIC * m_unkozaiMeanMotion
                                   / (m_semiLatusRectum * m_semiLatusRectum)
                                   * cosineInclination;

        m_rightAscensionDragCoefficient = 3.5 * betaSquared * j2NodalRate * m_dragCoefficient1;

        // Validate drag coefficients
        if (!std::isfinite (m_dragCoefficient1) || !std::isfinite (m_dragCoefficient2)
            || !std::isfinite (m_dragCoefficient3) || !std::isfinite (m_dragCoefficient4)
            || !std::isfinite (m_dragCoefficient5)
            || !std::isfinite (m_eccentricityDragCoefficient1)
            || !std::isfinite (m_eccentricityDragCoefficient2)
            || !std::isfinite (m_eccentricityDragCoefficient3)
            || !std::isfinite (m_meanMotionDragCoefficient1)
            || !std::isfinite (m_meanMotionDragCoefficient2)
            || !std::isfinite (m_meanMotionDragCoefficient3)
            || !std::isfinite (m_meanMotionDragCoefficient4)
            || !std::isfinite (m_argumentOfPerigeeDragCoefficient)
            || !std::isfinite (m_meanAnomalyDragCoefficient)
            || !std::isfinite (m_rightAscensionDragCoefficient))
        {
                std::cerr << "Invalid drag coefficients: Non-finite values detected!"
                          << std::endl;
                return false;
        }

        // Validate non-negative drag coefficients
        if (m_dragCoefficient2 < 0.0 || m_eccentricityDragCoefficient1 < 0.0
            || m_eccentricityDragCoefficient2 < 0.0 || m_eccentricityDragCoefficient3 < 0.0
            || m_meanMotionDragCoefficient2 < 0.0)
        {
                std::cerr << "Invalid drag coefficients: Negative values detected!"
                          << std::endl;
                return false;
        }

        return true;
}

bool sgp4PropagationData_t::calculateSecularCoefficients ()
{
        // Secular coefficients

        const double eccentricitySquared = m_eccentricity * m_eccentricity;

        const double cosineInclination = std::cos (m_inclination);
        const double cosineInclinationSquared = cosineInclination * cosineInclination;

        const double cosineInclinationFourth =
                cosineInclinationSquared * cosineInclinationSquared;

        const double beta = std::sqrt (1.0 - eccentricitySquared);

        const double inverseSemiLatusRectumSquared =
                1.0 / (m_semiLatusRectum * m_semiLatusRectum);

        const double con41 = 3.0 * cosineInclinationSquared - 1.0;
        const double con42 = 1.0 - 5.0 * cosineInclinationSquared;

        // Intermediate secular coefficients
        const double temp1 = 1.5 * wgs72::SECOND_ZONAL_HARMONIC * m_unkozaiMeanMotion
                             * inverseSemiLatusRectumSquared;

        const double temp2 =
                0.5 * temp1 * wgs72::SECOND_ZONAL_HARMONIC * inverseSemiLatusRectumSquared;

        const double temp3 = -0.46875 * wgs72::FOURTH_ZONAL_HARMONIC * m_unkozaiMeanMotion
                             * inverseSemiLatusRectumSquared * inverseSemiLatusRectumSquared;

        // Mean anomaly rate (mdot)
        m_meanAnomalyRate = m_unkozaiMeanMotion + 0.5 * temp1 * beta * con41
                            + 0.0625 * temp2 * beta
                                      * (13.0 - 78.0 * cosineInclinationSquared
                                         + 137.0 * cosineInclinationFourth);

        // Argument of perigee rate (argpdot)
        m_argumentOfPerigeeRate = -0.5 * temp1 * con42
                                  + 0.0625 * temp2
                                            * (7.0 - 114.0 * cosineInclinationSquared
                                               + 395.0 * cosineInclinationFourth)
                                  + temp3
                                            * (3.0 - 36.0 * cosineInclinationSquared
                                               + 49.0 * cosineInclinationFourth);

        // Right ascension rate (nodedot)
        m_rightAscensionRate = -temp1 * cosineInclination
                               + (0.5 * temp2 * (4.0 - 19.0 * cosineInclinationSquared)
                                  + 2.0 * temp3 * (3.0 - 7.0 * cosineInclinationSquared))
                                         * cosineInclination;

        // Validate secular coefficients
        if (!std::isfinite (m_meanAnomalyRate) || !std::isfinite (m_argumentOfPerigeeRate)
            || !std::isfinite (m_rightAscensionRate))
        {
                std::cerr << "Invalid secular coefficients!" << std::endl;
                return false;
        }

        return true;
}

bool sgp4PropagationData_t::calculateLongPeriodCoefficients ()
{
        // Long-period coefficients

        const double cosineInclination = std::cos (m_inclination);

        // Y coefficient (aycof)
        m_longPeriodCoefficientY =
                -0.5 * wgs72::RATIO_SECOND_THIRD_HARMONIC * std::sin (m_inclination);

        // X coefficient (xlcof)
        const double denominator = 1.0 + cosineInclination;
        const double minimumDenominator = 1.5e-12;

        m_longPeriodCoefficientX =
                -0.25 * wgs72::RATIO_SECOND_THIRD_HARMONIC * std::sin (m_inclination)
                * (3.0 + 5.0 * cosineInclination)
                / (std::abs (denominator) > minimumDenominator ? denominator
                                                               : minimumDenominator);

        // Validate long-period coefficients
        if (!std::isfinite (m_longPeriodCoefficientX)
            || !std::isfinite (m_longPeriodCoefficientY))
        {
                std::cerr << "Invalid long-period coefficients!" << std::endl;
                return false;
        }

        return true;
}

sgp4Propagator_t::sgp4Propagator_t ()
{
        std::cout << "Starting constructor for SPG4 propagator" << std::endl;
}

bool sgp4Propagator_t::initialize (
        const satellite_t &satellite_,
        std::unique_ptr<const propagationData_t> &propagationData_) const
{
        std::cout << "Starting initialize for SGP4 propagator" << std::endl;

        return true;
}

satelliteState_t sgp4Propagator_t::propagate (const propagationData_t &propagationData_,
                                              timePoint time_) const
{
        const auto *sgp4Data = dynamic_cast<const sgp4PropagationData_t *> (&propagationData_);

        if (sgp4Data == nullptr)
        {
                throw std::invalid_argument (
                        "Invalid propagation data supplied to SGP4 propagator!");
        }

        // Verify successful initialization
        if (!sgp4Data->m_isInitialized)
        {
                throw std::runtime_error ("SGP4 propagation data has not been initialized!");
        }

        // Calculate elapsed time since TLE epoch
        const double elapsedTimeMinutes =
                std::chrono::duration<double, std::ratio<60>> (time_ - sgp4Data->m_epoch)
                        .count ();

        if (!std::isfinite (elapsedTimeMinutes))
        {
                throw std::runtime_error (
                        "Invalid elapsed time calculated for SGP4 propagation!");
        }

        // Secular orbital element updates
        double updatedMeanAnomaly =
                sgp4Data->m_meanAnomaly + sgp4Data->m_meanAnomalyRate * elapsedTimeMinutes;

        double updatedArgumentOfPerigee =
                sgp4Data->m_argumentOfPerigee
                + sgp4Data->m_argumentOfPerigeeRate * elapsedTimeMinutes;

        double updatedRightAscensionOfAscendingNode =
                sgp4Data->m_rightAscensionOfAscendingNode
                + sgp4Data->m_rightAscensionRate * elapsedTimeMinutes;

        // Second-order ascending node correction
        const double elapsedTimeSquared = elapsedTimeMinutes * elapsedTimeMinutes;

        updatedRightAscensionOfAscendingNode +=
                sgp4Data->m_rightAscensionDragCoefficient * elapsedTimeSquared;

        double semiMajorAxisDragCorrection =
                1.0 - sgp4Data->m_dragCoefficient1 * elapsedTimeMinutes;

        double eccentricityDragCorrection =
                sgp4Data->m_bstar * sgp4Data->m_dragCoefficient4 * elapsedTimeMinutes;

        double meanLongitudeDragCorrection =
                sgp4Data->m_meanMotionDragCoefficient1 * elapsedTimeSquared;

        if (!sgp4Data->m_isSimplified)
        {
                const double elapsedTimeCubed = elapsedTimeSquared * elapsedTimeMinutes;

                const double elapsedTimeFourth = elapsedTimeCubed * elapsedTimeMinutes;

                semiMajorAxisDragCorrection -=
                        sgp4Data->m_eccentricityDragCoefficient1 * elapsedTimeSquared
                        + sgp4Data->m_eccentricityDragCoefficient2 * elapsedTimeCubed
                        + sgp4Data->m_eccentricityDragCoefficient3 * elapsedTimeFourth;

                meanLongitudeDragCorrection +=
                        sgp4Data->m_meanMotionDragCoefficient2 * elapsedTimeCubed
                        + elapsedTimeFourth
                                  * (sgp4Data->m_meanMotionDragCoefficient3
                                     + elapsedTimeMinutes
                                               * sgp4Data->m_meanMotionDragCoefficient4);
        }

        // Recover corrected semi-major axis
        double updatedSemiMajorAxis = sgp4Data->m_semiMajorAxis * semiMajorAxisDragCorrection
                                      * semiMajorAxisDragCorrection;

        // Recover corrected eccentricity
        double updatedEccentricity = sgp4Data->m_eccentricity - eccentricityDragCorrection;

        if (!std::isfinite (updatedSemiMajorAxis) || !std::isfinite (updatedEccentricity)
            || updatedSemiMajorAxis <= 0.0 || updatedEccentricity < 0.0
            || updatedEccentricity >= 1.0)
        {
                throw std::runtime_error (
                        "Invalid corrected orbital elements during SGP4 propagation!");
        }

        // Recover corrected mean motion
        double updatedMeanMotion = wgs72::XKE / std::pow (updatedSemiMajorAxis, 1.5);

        // Argument of perigee drag correction
        const double argumentOfPerigeeDragCorrection =
                sgp4Data->m_argumentOfPerigeeDragCoefficient * elapsedTimeMinutes;

        // Mean anomaly drag correction
        const double meanAnomalyDragIntermediate =
                1.0 + sgp4Data->m_eta * std::cos (updatedMeanAnomaly);

        const double meanAnomalyDragCorrection =
                sgp4Data->m_meanAnomalyDragCoefficient
                * (std::pow (meanAnomalyDragIntermediate, 3.0) - sgp4Data->m_delmo);

        // Combine argument of perigee and mean anomaly drag corrections

        const double combinedDragCorrection =
                argumentOfPerigeeDragCorrection + meanAnomalyDragCorrection;

        // Apply drag corrections
        updatedMeanAnomaly += combinedDragCorrection;
        updatedArgumentOfPerigee -= combinedDragCorrection;

        // Calculate corrected mean longitude
        double correctedMeanLongitude = updatedMeanAnomaly + updatedArgumentOfPerigee
                                        + updatedRightAscensionOfAscendingNode
                                        + meanLongitudeDragCorrection;

        // Calculate corrected argument of latitude
        double correctedArgumentOfLatitude =
                correctedMeanLongitude - updatedRightAscensionOfAscendingNode;

        // Normalize corrected mean longitude
        correctedMeanLongitude = std::fmod (correctedMeanLongitude, constants::TWO_PI);

        if (correctedMeanLongitude < 0.0)
        {
                correctedMeanLongitude += constants::TWO_PI;
        }

        // Normalize corrected mean latitude
        correctedArgumentOfLatitude =
                std::fmod (correctedArgumentOfLatitude, constants::TWO_PI);

        if (correctedArgumentOfLatitude < 0.0)
        {
                correctedArgumentOfLatitude += constants::TWO_PI;
        }

        // Calculate corrected eccentricity component Y
        double correctedEccentricityComponentY =
                updatedEccentricity * std::sin (updatedArgumentOfPerigee)
                + sgp4Data->m_longPeriodCoefficientY / updatedSemiMajorAxis;

        // Calculate corrected eccentricity component X
        double correctedEccentricityComponentX =
                updatedEccentricity * std::cos (updatedArgumentOfPerigee)
                + sgp4Data->m_longPeriodCoefficientX / updatedSemiMajorAxis;

        // Calculate corrected eccentricity magnitude
        const double correctedEccentricityMagnitude = std::sqrt (
                correctedEccentricityComponentX * correctedEccentricityComponentX
                + correctedEccentricityComponentY * correctedEccentricityComponentY);

        // Calculate initial eccentric longitude
        const double initialEccentricLongitude = correctedArgumentOfLatitude;

        // Solve Kepler's equation
        const double eccentricLongitude =
                solveKeplersEquation (initialEccentricLongitude,
                                      correctedEccentricityComponentX,
                                      correctedEccentricityComponentY);

        // Calculate eccentricity times cosine of eccentric longitude
        const double eccentricityCosineComponent =
                correctedEccentricityComponentX * std::cos (eccentricLongitude)
                + correctedEccentricityComponentY * std::sin (eccentricLongitude);

        // Calculate eccentricity times sine of eccentric longitude
        const double eccentricitySineComponent =
                correctedEccentricityComponentX * std::sin (eccentricLongitude)
                - correctedEccentricityComponentY * std::cos (eccentricLongitude);

        // Calculate squared corrected eccentricity
        const double correctedEccentricitySquared =
                correctedEccentricityComponentX * correctedEccentricityComponentX
                + correctedEccentricityComponentY * correctedEccentricityComponentY;

        // Calculate corrected semi-latus rectum
        const double correctedSemiLatusRectum =
                updatedSemiMajorAxis * (1.0 - correctedEccentricitySquared);

        // Calculate corrected orbital radius
        const double correctedOrbitalRadius =
                updatedSemiMajorAxis * (1.0 - eccentricityCosineComponent);

        // Calculate reciprocal of corrected semi-latus rectum
        const double inverseCorrectedSemiLatusRectum = 1.0 / correctedSemiLatusRectum;

        // Calculate radial velocity-related intermediate
        const double radialVelocityIntermediate = std::sqrt (updatedSemiMajorAxis)
                                                  * eccentricitySineComponent
                                                  * inverseCorrectedSemiLatusRectum;

        // Calculate transverse velocity-related intermediate
        const double transverseVelocityIntermediate =
                std::sqrt (correctedSemiLatusRectum) / correctedOrbitalRadius;

        // Calculate inclination trigonometric values
        const double sineInclination = std::sin (sgp4Data->m_inclination);
        const double cosineInclination = std::cos (sgp4Data->m_inclination);

        const double sineInclinationSquared = sineInclination * sineInclination;
        const double cosineInclinationSquared = cosineInclination * cosineInclination;

        // Calculate short-period J2 correction factors
        const double shortPeriodCorrectionFactor1 =
                1.5 * wgs72::SECOND_ZONAL_HARMONIC * inverseCorrectedSemiLatusRectum;

        const double shortPeriodCorrectionFactor2 =
                shortPeriodCorrectionFactor1 * inverseCorrectedSemiLatusRectum;

        // Calculate double argument of latitude
        const double doubleArgumentOfLatitude = 2.0 * correctedArgumentOfLatitude;

        // Calculate short-period argument of latitude correction
        const double argumentOfLatitudeCorrection = 0.25 * shortPeriodCorrectionFactor2
                                                    * std::sin (doubleArgumentOfLatitude)
                                                    * (7.0 * cosineInclinationSquared - 1.0);

        // Calculate corrected argument of latitude
        const double shortPeriodCorrectedArgumentOfLatitude =
                correctedArgumentOfLatitude - argumentOfLatitudeCorrection;

        // Calculate short-period right ascension correction
        const double rightAscensionCorrection = 1.75 * shortPeriodCorrectionFactor2
                                                * cosineInclination
                                                * std::sin (doubleArgumentOfLatitude);

        // Calculate short-period corrected right ascension
        const double shortPeriodCorrectedRightAscension =
                updatedRightAscensionOfAscendingNode + rightAscensionCorrection;

        // Calculate short-period inclination correction
        const double inclinationCorrection = 1.5 * shortPeriodCorrectionFactor2
                                             * cosineInclination * sineInclination
                                             * std::cos (doubleArgumentOfLatitude);

        // Calculate short-period corrected inclination
        const double shortPeriodCorrectedInclination =
                sgp4Data->m_inclination + inclinationCorrection;

        // Calculate beta
        const double correctedBeta = std::sqrt (1.0 - correctedEccentricitySquared);

        // Calculate double argument of latitude cosine
        const double cosineDoubleArgumentOfLatitude = std::cos (doubleArgumentOfLatitude);

        // Calculate short-period corrected orbital radius
        const double shortPeriodCorrectedOrbitalRadius =
                correctedOrbitalRadius
                        * (1.0
                           - 1.5 * shortPeriodCorrectionFactor2 * correctedBeta
                                     * cosineDoubleArgumentOfLatitude)
                + 0.5 * shortPeriodCorrectionFactor1 * sineInclinationSquared
                          * cosineDoubleArgumentOfLatitude;

        // Calculate short-period corrected radial velocity intermediate
        const double shortPeriodCorrectedRadialVelocity =
                radialVelocityIntermediate
                - updatedMeanMotion * shortPeriodCorrectionFactor1 * sineInclinationSquared
                          * std::sin (doubleArgumentOfLatitude);

        // Calculate short-period corrected transverse velocity intermediate
        const double shortPeriodCorrectedTransverseVelocity =
                transverseVelocityIntermediate
                + updatedMeanMotion * shortPeriodCorrectionFactor1
                          * (sineInclinationSquared * cosineDoubleArgumentOfLatitude
                             + 1.5 * cosineInclinationSquared);

        // Calculate trigonometric values for corrected orbital elements
        const double sineCorrectedArgumentOfLatitude =
                std::sin (shortPeriodCorrectedArgumentOfLatitude);
        const double cosineCorrectedArgumentOfLatitude =
                std::cos (shortPeriodCorrectedArgumentOfLatitude);
        const double sineCorrectedRightAscension =
                std::sin (shortPeriodCorrectedRightAscension);
        const double cosineCorrectedRightAscension =
                std::cos (shortPeriodCorrectedRightAscension);
        const double sineCorrectedInclination = std::sin (shortPeriodCorrectedInclination);
        const double cosineCorrectedInclination = std::cos (shortPeriodCorrectedInclination);

        const vector3_t radialDirection{
                -sineCorrectedRightAscension * cosineCorrectedInclination
                                * sineCorrectedArgumentOfLatitude
                        + cosineCorrectedRightAscension * cosineCorrectedArgumentOfLatitude,

                cosineCorrectedRightAscension * cosineCorrectedInclination
                                * sineCorrectedArgumentOfLatitude
                        + sineCorrectedRightAscension * cosineCorrectedArgumentOfLatitude,

                sineCorrectedInclination * sineCorrectedArgumentOfLatitude};

        const vector3_t transverseDirection{
                -sineCorrectedRightAscension * cosineCorrectedInclination
                                * cosineCorrectedArgumentOfLatitude
                        - cosineCorrectedRightAscension * sineCorrectedArgumentOfLatitude,

                cosineCorrectedRightAscension * cosineCorrectedInclination
                                * cosineCorrectedArgumentOfLatitude
                        - sineCorrectedRightAscension * sineCorrectedArgumentOfLatitude,

                sineCorrectedInclination * cosineCorrectedArgumentOfLatitude};

        // Calculate TEME position
        const double positionMagnitude =
                wgs72::EARTH_EQUATORIAL_RADIUS * shortPeriodCorrectedOrbitalRadius;

        const vector3_t temePosition{positionMagnitude * radialDirection.x,
                                     positionMagnitude * radialDirection.y,
                                     positionMagnitude * radialDirection.z};

        // Calculate TEME velocity scaling factor
        const double velocityScalingFactor =
                wgs72::EARTH_EQUATORIAL_RADIUS * wgs72::XKE / 60.0;

        // Calculate radial velocity vector
        const vector3_t radialVelocity{shortPeriodCorrectedRadialVelocity * radialDirection.x,
                                       shortPeriodCorrectedRadialVelocity * radialDirection.y,
                                       shortPeriodCorrectedRadialVelocity * radialDirection.z};

        // Calculate transverse velocity vector
        const vector3_t transverseVelocity{
                shortPeriodCorrectedTransverseVelocity * transverseDirection.x,
                shortPeriodCorrectedTransverseVelocity * transverseDirection.y,
                shortPeriodCorrectedTransverseVelocity * transverseDirection.z};

        // Calculate TEME velocity
        const vector3_t temeVelocity{
                velocityScalingFactor * (radialVelocity.x + transverseVelocity.x),

                velocityScalingFactor * (radialVelocity.y + transverseVelocity.y),

                velocityScalingFactor * (radialVelocity.z + transverseVelocity.z)};

        // Construct TEME state
        temeState_t temeState;
        temeState.position = temePosition;
        temeState.velocity = temeVelocity;

        satelliteState_t satelliteState =
                coordinateTransform::temeToSatelliteState (temeState, time_);

        /*std::cout << "Elapsed time: " << elapsedTimeMinutes << " minutes" << std::endl;
        std::cout << "updatedMeanAnomaly: " << updatedMeanAnomaly << std::endl;
        std::cout << "updatedArgumentOfPerigee: " << updatedArgumentOfPerigee << std::endl;
        std::cout << "updatedRightAscensionOfAscendingNode: "
                  << updatedRightAscensionOfAscendingNode << std::endl;
        std::cout << "semiMajorAxisDragCorrection: " << semiMajorAxisDragCorrection
                  << std::endl;
        std::cout << "eccentricityDragCorrection: " << eccentricityDragCorrection << std::endl;
        std::cout << "meanLongitudeDragCorrection: " << meanLongitudeDragCorrection
                  << std::endl;
        std::cout << "Updated semi-major axis: " << updatedSemiMajorAxis << " Earth radii"
                  << std::endl;
        std::cout << "Updated eccentricity: " << updatedEccentricity << std::endl;
        std::cout << "Updated mean motion: " << updatedMeanMotion << " rad/min" << std::endl;
        std::cout << "Argument of perigee drag correction: " << argumentOfPerigeeDragCorrection
                  << std::endl;
        std::cout << "Mean anomaly drag correction: " << meanAnomalyDragCorrection
                  << std::endl;
        std::cout << "Combined drag correction: " << combinedDragCorrection << std::endl;
        std::cout << "Corrected mean anomaly: " << updatedMeanAnomaly << std::endl;
        std::cout << "Corrected argument of perigee: " << updatedArgumentOfPerigee
                  << std::endl;
        std::cout << "Corrected Mean Longitude (normalized): " << correctedMeanLongitude
                  << std::endl;
        std::cout << "Corrected Argument of Latitude (normalized): "
                  << correctedArgumentOfLatitude << std::endl;
        std::cout << "correctedEccentricityComponentY: " << correctedEccentricityComponentY
                  << std::endl;
        std::cout << "correctedEccentricityComponentX: " << correctedEccentricityComponentX
                  << std::endl;
        std::cout << "Corrected eccentricity magnitude: " << correctedEccentricityMagnitude
                  << std::endl;
        std::cout << "Initial eccentric longitude: " << initialEccentricLongitude << std::endl;
        std::cout << "Solved eccentric longitude: " << eccentricLongitude << std::endl;
        std::cout << "Eccentricity cosine component: " << eccentricityCosineComponent
                  << std::endl;
        std::cout << "Eccentricity sine component: " << eccentricitySineComponent << std::endl;
        std::cout << "Corrected eccentricity squared: " << correctedEccentricitySquared
                  << std::endl;
        std::cout << "Corrected semi-latus rectum: " << correctedSemiLatusRectum << std::endl;
        std::cout << "Corrected orbital radius: " << correctedOrbitalRadius << std::endl;
        std::cout << "Inverse corrected semi-latus rectum: " << inverseCorrectedSemiLatusRectum
                  << std::endl;
        std::cout << "Radial velocity intermediate: " << radialVelocityIntermediate
                  << std::endl;
        std::cout << "Transverse velocity intermediate: " << transverseVelocityIntermediate
                  << std::endl;
        std::cout << "Short-period correction factor 1: " << shortPeriodCorrectionFactor1
                  << std::endl;
        std::cout << "Short-period correction factor 2: " << shortPeriodCorrectionFactor2
                  << std::endl;
        std::cout << "Argument of latitude correction: " << argumentOfLatitudeCorrection
                  << std::endl;
        std::cout << "Short-period corrected argument of latitude: "
                  << shortPeriodCorrectedArgumentOfLatitude << std::endl;
        std::cout << "Right ascension correction: " << rightAscensionCorrection << std::endl;
        std::cout << "Short-period corrected right ascension: "
                  << shortPeriodCorrectedRightAscension << std::endl;
        std::cout << "Inclination correction: " << inclinationCorrection << std::endl;
        std::cout << "Short-period corrected inclination: " << shortPeriodCorrectedInclination
                  << std::endl;
        std::cout << "correctedBeta: " << correctedBeta << std::endl;
        std::cout << "cosineDoubleArgumentOfLatitude: " << cosineDoubleArgumentOfLatitude
                  << std::endl;
        std::cout << "shortPeriodCorrectedOrbitalRadius: " << shortPeriodCorrectedOrbitalRadius
                  << std::endl;
        std::cout << "shortPeriodCorrectedRadialVelocity: "
                  << shortPeriodCorrectedRadialVelocity << std::endl;
        std::cout << "shortPeriodCorrectedTransverseVelocity: "
                  << shortPeriodCorrectedTransverseVelocity << std::endl;*/

        /*std::cout << "\n--- TEME State ---" << std::endl;
        std::cout << "Position X: " << temePosition.x << " km" << std::endl;
        std::cout << "Position Y: " << temePosition.y << " km" << std::endl;
        std::cout << "Position Z: " << temePosition.z << " km" << std::endl;

        std::cout << "Velocity X: " << temeVelocity.x << " km/s" << std::endl;
        std::cout << "Velocity Y: " << temeVelocity.y << " km/s" << std::endl;
        std::cout << "Velocity Z: " << temeVelocity.z << " km/s" << std::endl;

        std::cout << "\n--- Satellite State (ECEF position + vel) ---" << std::endl;
        std::cout << "Position X: " << satelliteState.position.coordinates.x << std::endl;
        std::cout << "Position Y: " << satelliteState.position.coordinates.y << std::endl;
        std::cout << "Position Z: " << satelliteState.position.coordinates.z << std::endl;

        std::cout << "Velocity X: " << satelliteState.velocity.x << " km/s" << std::endl;
        std::cout << "Velocity Y: " << satelliteState.velocity.y << " km/s" << std::endl;
        std::cout << "Velocity Z: " << satelliteState.velocity.z << " km/s" << std::endl;*/

        return satelliteState;
}

double sgp4Propagator_t::solveKeplersEquation (double initialEccentricLongitude_,
                                               double eccentricityComponentX_,
                                               double eccentricityComponentY_) const
{
        // Initialize eccentric longitude
        double eccentricLongitude = initialEccentricLongitude_;

        // Solve Kepler's equation using Newton-Raphson iteration
        for (int iteration = 0; iteration < 10; ++iteration)
        {
                const double sineEccentricLongitude = std::sin (eccentricLongitude);

                const double cosineEccentricLongitude = std::cos (eccentricLongitude);

                const double keplerEquationError =
                        initialEccentricLongitude_
                        - eccentricityComponentY_ * cosineEccentricLongitude
                        + eccentricityComponentX_ * sineEccentricLongitude
                        - eccentricLongitude;

                const double keplerEquationDerivative =
                        1.0 - eccentricityComponentY_ * sineEccentricLongitude
                        - eccentricityComponentX_ * cosineEccentricLongitude;

                double eccentricLongitudeCorrection =
                        keplerEquationError / keplerEquationDerivative;

                // Limit the correction for numerical stability
                eccentricLongitudeCorrection =
                        std::clamp (eccentricLongitudeCorrection, -0.95, 0.95);

                eccentricLongitude += eccentricLongitudeCorrection;

                // Check convergence
                if (std::abs (eccentricLongitudeCorrection) < 1.0e-12)
                {
                        break;
                }
        }

        return eccentricLongitude;
}