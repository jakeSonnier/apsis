#include "satellite.H"
#include "satelliteState.H"

#include <chrono>
#include <memory>

using timePoint = std::chrono::system_clock::time_point;

class propagationData_t
{
public:
        virtual ~propagationData_t () = default;
};

class orbitPropagator_t
{
public:
        virtual ~orbitPropagator_t () = default;

        virtual bool initialize (
                const satellite_t &satellite_,
                std::unique_ptr<const propagationData_t> &propagationData_) const = 0;

        virtual satelliteState_t propagate (const propagationData_t &propagationData_,
                                            timePoint time_) const = 0;
};