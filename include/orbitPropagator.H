#include "satellite.H"
#include "satelliteState.H"

#include <chrono>

using timePoint = std::chrono::system_clock::time_point;

class orbitPropagator_t
{
public:
        virtual ~orbitPropagator_t () = default;

        virtual satelliteState_t propagate (const satellite_t &satellite,
                                            timePoint time) const = 0;
};