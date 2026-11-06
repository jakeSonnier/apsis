#include <cmath>

#pragma once

namespace wgs72
{
static constexpr double EARTH_GRAVITATIONAL_PARAMETER = 398600.8;

static constexpr double EARTH_EQUITORIAL_RADIUS = 6378.135;

static constexpr double SECOND_ZONAL_HARMONIC = 0.001082616;

static constexpr double THIRD_ZONAL_HARMONIC = -0.00000253881;

static constexpr double FOUTH_ZONAL_HARMONIC = -0.00000165597;

static constexpr double XKE =
        60.0
        / std::sqrt (std::pow (EARTH_EQUITORIAL_RADIUS, 3) / EARTH_GRAVITATIONAL_PARAMETER);

static constexpr double TUMIN = 1.0 / XKE;

static constexpr double RATIO_SECOND_THIRD_HARMONIC =
        SECOND_ZONAL_HARMONIC / THIRD_ZONAL_HARMONIC;
}