#pragma once
#include <Standards.hpp>

// Base class for a system that runs once per tick, in registration order.
// Subclass and register with TickLoop::registerSystem() to add tick-driven
// behavior (Keep Alive today; entity AI/block updates/world simulation later)
// without TickLoop needing to know about it.
class TickSystem {
    public:
        virtual ~TickSystem() = default;
        virtual void onTick(Int64 tickCount) = 0;
        virtual string getName() const = 0;
};