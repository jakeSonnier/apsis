#include "coordinateTransform.H"
#include "groundStation.H"

groundStation_t::groundStation_t (double latitude_,
                                  double longitude_,
                                  double altitude_,
                                  double minimumElevation_)
        : m_minimumElevation (minimumElevation_)
{
        m_geographicPosition.latitude = latitude_;
        m_geographicPosition.longitude = longitude_;
        m_geographicPosition.altitude = altitude_;

        m_ecefPosition = coordinateTransform::geographicToEcef (m_geographicPosition);
}

geographicPosition_t groundStation_t::geographicPosition () const
{
        return m_geographicPosition;
}

ecefPosition_t groundStation_t::ecefPosition () const
{
        return m_ecefPosition;
}

double groundStation_t::minimumElevation () const
{
        return m_minimumElevation;
}