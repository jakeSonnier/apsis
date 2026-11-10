#include "groundStation.H"
#include "orbitPropagator.H"

#include <memory>

#pragma once

using timePoint = std::chrono::system_clock::time_point;

namespace passPrediction
{
struct validSatellitePass_t
{
        timePoint acquisitionOfSignal;
        timePoint lossOfSignal;
        double totalPassTime;
        double maximumElevation;
};

std::vector<validSatellitePass_t> calculateValidPasses (
        groundStation_t const &groundStation_,
        propagationData_t const &propagationData_,
        int searchHours_);
}