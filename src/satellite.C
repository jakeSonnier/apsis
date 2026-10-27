#include "satellite.H"

#include <iomanip>
#include <iostream>
#include <sstream>

orbit_t::~orbit_t ()
{
}

orbit_t::orbit_t (double argumentOfPerigee_,
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
                  double rightAscensionOfAscendingNode_)
        : m_argumentOfPerigee (argumentOfPerigee_)
        , m_bstar (bstar_)
        , m_eccentricity (eccentricity_)
        , m_elementNumber (elementNumber_)
        , m_ephemerisType (ephemerisType_)
        , m_epochDay (epochDay_)
        , m_epochYear (epochYear_)
        , m_inclination (inclination_)
        , m_meanAnomaly (meanAnomaly_)
        , m_meanMotion (meanMotion_)
        , m_revolutionNumber (revolutionNumber_)
        , m_rightAscensionOfAscendingNode (rightAscensionOfAscendingNode_)
{
}

double orbit_t::argumentOfPerigee () const
{
        return m_argumentOfPerigee;
}

double orbit_t::bstar () const
{
        return m_bstar;
}

double orbit_t::eccentricity () const
{
        return m_eccentricity;
}

int orbit_t::elementNumber () const
{
        return m_elementNumber;
}

char orbit_t::ephemerisType () const
{
        return m_ephemerisType;
}

int orbit_t::epochDay () const
{
        return m_epochDay;
}

int orbit_t::epochYear () const
{
        return m_epochYear;
}

double orbit_t::inclination () const
{
        return m_inclination;
}

double orbit_t::meanAnomaly () const
{
        return m_meanAnomaly;
}

double orbit_t::meanMotion () const
{
        return m_meanMotion;
}

int orbit_t::revolutionNumber () const
{
        return m_revolutionNumber;
}

double orbit_t::rightAscensionOfAscendingNode () const
{
        return m_rightAscensionOfAscendingNode;
}

satellite_t::~satellite_t ()
{
}

satellite_t::satellite_t (char classification_,
                          std::string internationalDesignator_,
                          std::string name_,
                          int number_,
                          orbit_t orbit_)
        : m_classification (classification_)
        , m_internationalDesignator (internationalDesignator_)
        , m_name (name_)
        , m_number (number_)
        , m_orbit (orbit_)
{
}

std::string satellite_t::toString () const
{
        std::ostringstream out;

        out << std::fixed << std::setprecision (6);

        out << "Satellite: " << name () << '\n'
            << "----------------\n"
            << "Catalog number: " << number () << '\n'
            << "Classification: " << classification () << '\n'
            << "International designator: " << internationalDesignator () << '\n'

            << "\nOrbital Elements\n"
            << "----------------\n"
            << "Epoch year: " << orbit ().epochYear () << '\n'
            << "Epoch day: " << orbit ().epochDay () << '\n'
            << "Inclination: " << orbit ().inclination () << " degrees\n"
            << "RAAN: " << orbit ().rightAscensionOfAscendingNode () << " degrees\n"
            << "Eccentricity: " << orbit ().eccentricity () << '\n'
            << "Argument of perigee: " << orbit ().argumentOfPerigee () << " degrees\n"
            << "Mean anomaly: " << orbit ().meanAnomaly () << " degrees\n"
            << "Mean motion: " << orbit ().meanMotion () << " revolutions/day\n";

        return out.str ();
}

char satellite_t::classification () const
{
        return m_classification;
}

std::string satellite_t::internationalDesignator () const
{
        return m_internationalDesignator;
}

std::string satellite_t::name () const
{
        return m_name;
}

int satellite_t::number () const
{
        return m_number;
}

orbit_t satellite_t::orbit () const
{
        return m_orbit;
}