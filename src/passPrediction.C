#include "coordinateTransform.H"
#include "passPrediction.H"
#include "sgp4Propagator.H"
#include "states.H"
#include "visibility.H"

#include <iostream>

std::vector<passPrediction::validSatellitePass_t> passPrediction::calculateValidPasses (
        groundStation_t const &groundStation_,
        propagationData_t const &propagationData_,
        int searchHours_)
{
        const timePoint startTime = std::chrono::system_clock::now ();
        const timePoint endTime = startTime + std::chrono::hours (searchHours_);
        bool calculatingAOS = false;
        timePoint currentAosTime;
        timePoint currentLosTime;
        double maximumElevation = 0;
        double cumulativePassTime = 0;

        std::vector<passPrediction::validSatellitePass_t> validPasses;

        sgp4Propagator_t sgp4Propagator;

        const sgp4PropagationData_t &sgp4PropagationData =
                dynamic_cast<const sgp4PropagationData_t &> (propagationData_);

        for (timePoint time = startTime; time <= endTime; time += std::chrono::minutes (1))
        {
                const satelliteState_t sgp4SatelliteState =
                        sgp4Propagator.propagate (sgp4PropagationData, time);

                const geographicPosition_t sgp4GeographicPosition =
                        coordinateTransform::ecefToGeographic (sgp4SatelliteState.position);

                const double sgp4LatitudeDegrees =
                        sgp4GeographicPosition.latitude * (180.0 / std::numbers::pi);

                const double sgp4LongitudeDegrees =
                        sgp4GeographicPosition.longitude * (180.0 / std::numbers::pi);

                visibilityResult_t visibilityResult =
                        calculateVisibility (sgp4SatelliteState.position, groundStation_);

                if (visibilityResult.visible)
                {
                        maximumElevation = 0.0;
                        cumulativePassTime = 0.0;
                        currentAosTime = time;
                        calculatingAOS = true;

                        // Back up to calculate AOS to the second
                        while (calculatingAOS)
                        {
                                const satelliteState_t preciseAosState =
                                        sgp4Propagator.propagate (sgp4PropagationData, time);

                                visibilityResult_t preciseVisibilityResultForAos =
                                        calculateVisibility (
                                                preciseAosState.position, groundStation_);

                                if (preciseVisibilityResultForAos.visible)
                                {
                                        currentAosTime = time;
                                        time -= std::chrono::seconds (1);
                                }
                                else
                                {
                                        calculatingAOS = false;
                                        break;
                                }
                        }

                        time += std::chrono::seconds (1);

                        // Calculate pass, keeping track of max elevation second by
                        // second
                        while (true)
                        {
                                const satelliteState_t passState =
                                        sgp4Propagator.propagate (sgp4PropagationData, time);

                                visibilityResult_t preciseVisibilityResultPass =
                                        calculateVisibility (
                                                passState.position, groundStation_);

                                if (preciseVisibilityResultPass.visible)
                                {
                                        currentLosTime = time;

                                        if (preciseVisibilityResultPass.elevation
                                            > maximumElevation)
                                        {
                                                maximumElevation =
                                                        preciseVisibilityResultPass.elevation;
                                        }

                                        cumulativePassTime++;
                                }
                                else
                                {
                                        passPrediction::validSatellitePass_t pass;
                                        pass.acquisitionOfSignal = currentAosTime;
                                        pass.lossOfSignal = currentLosTime;
                                        pass.totalPassTime = cumulativePassTime;
                                        pass.maximumElevation = maximumElevation;
                                        validPasses.push_back (pass);

                                        break;
                                }

                                time += std::chrono::seconds (1);
                        }
                }
        }

        return validPasses;
}