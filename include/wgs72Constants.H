#include <cmath>

#pragma once

namespace wgs72
{
// Gravity model constants
static constexpr double EARTH_GRAVITATIONAL_PARAMETER = 398600.8;
static constexpr double EARTH_EQUATORIAL_RADIUS = 6378.135;
static constexpr double SECOND_ZONAL_HARMONIC = 0.001082616;
static constexpr double THIRD_ZONAL_HARMONIC = -0.00000253881;
static constexpr double FOURTH_ZONAL_HARMONIC = -0.00000165597;

static constexpr double XKE =
        60.0
        / std::sqrt (std::pow (EARTH_EQUATORIAL_RADIUS, 3.0) / EARTH_GRAVITATIONAL_PARAMETER);

static constexpr double TUMIN = 1.0 / XKE;

static constexpr double RATIO_SECOND_THIRD_HARMONIC =
        THIRD_ZONAL_HARMONIC / SECOND_ZONAL_HARMONIC;

// Atmospheric model constants
static constexpr double ATMOSPHERIC_REFERENCE_ALTITUDE = 78.0;   // km
static constexpr double ATMOSPHERIC_TRANSITION_ALTITUDE = 120.0; // km
static constexpr double LOW_PERIGEE_THRESHOLD = 156.0;           // km
static constexpr double VERY_LOW_PERIGEE_THRESHOLD = 98.0;       // km
static constexpr double MINIMUM_REFERENCE_ALTITUDE = 20.0;       // km

static constexpr double ATMOSPHERIC_REFERENCE_DISTANCE =
        1.0 + ATMOSPHERIC_REFERENCE_ALTITUDE / EARTH_EQUATORIAL_RADIUS;

static constexpr double ATMOSPHERIC_DENSITY_PARAMETER =
        std::pow ((ATMOSPHERIC_TRANSITION_ALTITUDE - ATMOSPHERIC_REFERENCE_ALTITUDE)
                          / EARTH_EQUATORIAL_RADIUS,
                  4.0);
}