#include <string>

#pragma once

// \brief ...
class orbit_t
{
public:
        ~orbit_t ();

        orbit_t (double argumentOfPerigee_,
                 double bstar_,
                 double eccentricity_,
                 int elementNumber_,
                 char ephemerisType_,
                 int epochDay_,
                 int epochYear_,
                 double inclination_,
                 double meanAnomaly_,
                 double meanMotion_,
                 int revolutionNumber_,
                 double rightAscensionOfAscendingNode_);

        /// \brief Returns the argument of perigee.
        /// \note Angle measured from the ascending node to perigee in the orbital plane.
        /// \unit Degrees
        double argumentOfPerigee () const;

        /// \brief Returns the BSTAR drag coefficient.
        /// \note Empirical parameter used by the SGP4 orbital propagation model to
        ///       account for atmospheric drag. Its value is defined by the TLE format
        ///       and is not a conventional drag coefficient in SI units.
        /// \unit Reciprocal Earth radii (1/Earth radii)
        double bstar () const;

        /// \brief Returns the orbital eccentricity.
        /// \note Dimensionless measure of the deviation of the orbit from a circle.
        ///       A value of 0 represents a circular orbit; values between 0 and 1
        ///       represent elliptical orbits.
        double eccentricity () const;

        /// \brief Returns the element set number.
        /// \note Identifies the element set in the source TLE data.
        ///       It is assigned by the data provider and is not a physical orbital parameter.
        int elementNumber () const;

        /// \brief Returns the ephemeris type.
        /// \note Indicates the ephemeris type specified in the TLE data.
        ///       A value of 0 is conventionally used for SGP4/SDP4-compatible TLEs.
        char ephemerisType () const;

        /// \brief Returns the epoch day.
        /// \note Day of the year, including the fractional part representing the time
        ///       of day, as encoded in the TLE epoch field. The returned integer
        ///       contains only the whole-number day if the value is stored as an integer.
        /// \unit Days of year (integer day number)
        int epochDay () const;

        /// \brief Returns the epoch year.
        /// \note Two-digit year from the TLE epoch field. The conventional TLE
        ///       interpretation maps values 57-99 to 1957-1999 and 00-56 to 2000-2056.
        /// \unit Years (two-digit year)
        int epochYear () const;

        /// \brief Returns the orbital inclination.
        /// \note Angle between the orbital plane and the Earth's equatorial plane.
        /// \unit Degrees
        double inclination () const;

        /// \brief Returns the mean anomaly.
        /// \note Orbital angle measured from perigee to the satellite's mean position,
        ///       increasing in the direction of orbital motion.
        /// \unit Degrees
        double meanAnomaly () const;

        /// \brief Returns the mean motion.
        /// \note Average angular orbital rate expressed as the number of revolutions
        ///       completed per mean solar day.
        /// \unit Revolutions per day (rev/day)
        double meanMotion () const;

        /// \brief Returns the revolution number at the epoch.
        /// \note Number of revolutions completed by the satellite at the TLE epoch,
        ///       as specified in the source TLE data.
        /// \unit Revolutions (integer count)
        int revolutionNumber () const;

        /// \brief Returns the right ascension of the ascending node (RAAN).
        /// \note Angle measured eastward from the vernal equinox to the ascending node
        ///       in the Earth's equatorial plane.
        /// \unit Degrees
        double rightAscensionOfAscendingNode () const;

private:
        double m_argumentOfPerigee;
        double m_bstar;
        double m_eccentricity;
        int m_elementNumber;
        char m_ephemerisType;
        int m_epochDay;
        int m_epochYear;
        double m_inclination;
        double m_meanAnomaly;
        double m_meanMotion;
        int m_revolutionNumber;
        double m_rightAscensionOfAscendingNode;
};

// \brief ...
class satellite_t
{
public:
        ~satellite_t ();

        satellite_t (char classification_,
                     std::string internationalDesignator_,
                     std::string name_,
                     int number_,
                     orbit_t orbit_);

        std::string toString () const;

        /// \brief Returns the classification of the satellite.
        /// \note Indicates the security classification assigned to the satellite
        ///       in the TLE data. Common values are 'U' for unclassified,
        ///       'C' for classified, and 'S' for secret.
        /// \unit Character (classification code)
        char classification () const;

        /// \brief Returns the international designator of the satellite.
        /// \note Identifies the satellite's launch year, launch number, and piece
        ///       designation, as assigned in the international satellite catalog.
        ///       For TLE data, this typically follows the format YYNNNPPP.
        /// \unit None (text identifier)
        std::string internationalDesignator () const;

        /// \brief Returns the name of the satellite.
        /// \note Provides the human-readable name associated with the satellite
        ///       in the source data.
        /// \unit None (text)
        std::string name () const;

        /// \brief Returns the satellite catalog number.
        /// \note Identifies the satellite using its catalog number, commonly the
        ///       NORAD catalog number in TLE data.
        /// \unit None (integer identifier)
        int number () const;

        /// \brief Returns the orbital elements of the satellite.
        /// \note Provides the orbital parameters used to describe the satellite's
        ///       orbit, as represented by the orbit_t type.
        /// \unit Depends on the orbit_t type definition
        orbit_t orbit () const;

private:
        char m_classification;
        std::string m_internationalDesignator;
        std::string m_name;
        int m_number;

        orbit_t m_orbit;
};