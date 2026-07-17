#pragma once
#include <TickSystem.hpp>

// Sends a Keep Alive packet to every Config/Play-state connection every
// KEEP_ALIVE_INTERVAL_TICKS ticks, so idle real clients don't hit their own
// ~20s no-traffic timeout. One global tick-modulo gate for every connection,
// not per-connection staggering (vanilla staggers this; not needed yet).
class KeepAliveSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
    private:
        static constexpr Int64 KEEP_ALIVE_INTERVAL_TICKS = 200; // 10s @ 20 TPS
};