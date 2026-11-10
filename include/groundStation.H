#include "states.H"

#pragma once

class groundStation_t
{
public:
        groundStation_t (double latitude_,
                         double longitude_,
                         double altitude_,
                         double minimumElevation_);

        /// \brief Returns the geographic position of the ground station. Assumes
        /// stationary ground station
        /// \units Lat/Lon/Alt
        geographicPosition_t geographicPosition () const;
        /// \brief Returns the calculated ECEF position of the ground station. Assumes
        /// stationary ground station
        /// \units Meters
        ecefPosition_t ecefPosition () const;

        /// \brief Returns the minimum elevation angle for satellite visibility.
        /// \unit Degrees
        double minimumElevation () const;

private:
        geographicPosition_t m_geographicPosition;
        ecefPosition_t m_ecefPosition;

        double m_minimumElevation;
};