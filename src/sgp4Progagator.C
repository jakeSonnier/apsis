#include "sgp4Propagator.H"

#include <iostream>

bool sgp4PropagationData_t::initialize (const satellite_t &satellite_) const
{
        std::cout << "Starting initialize for SGP4 propagation data" << std::endl;

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
        satelliteState_t satelliteState;

        std::cout << "Starting SGP4 propagation" << std::endl;

        return satelliteState;
}