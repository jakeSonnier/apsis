#include "orbitPropagator.H"

class sgp4Propagator_t final : public orbitPropagator_t
{
public:
        sgp4Propagator_t ();

        satelliteState_t propagate (const satellite_t &satellite,
                                    timePoint time) const override;
};