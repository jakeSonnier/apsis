#include "orbitPropagator.H"
#include "satellite.H"

#pragma once

class sgp4PropagationData_t final : public propagationData_t
{
public:
        bool initialize (const satellite_t &satellite_) const;
};

class sgp4Propagator_t final : public orbitPropagator_t
{
public:
        sgp4Propagator_t ();

        bool initialize (
                const satellite_t &satellite_,
                std::unique_ptr<const propagationData_t> &propagationData_) const override;

        satelliteState_t propagate (const propagationData_t &propagationData_,
                                    timePoint time_) const override;
};