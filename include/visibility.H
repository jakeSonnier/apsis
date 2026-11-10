#include "groundStation.H"
#include "states.H"

#include <ostream>

#pragma once

struct visibilityResult_t
{
        bool visible;
        double elevation;
        double azimuth;
        double range;
};

std::string toString (visibilityResult_t result_);

visibilityResult_t calculateVisibility (const ecefPosition_t &satellitePosition_,
                                        const groundStation_t &groundStation_);