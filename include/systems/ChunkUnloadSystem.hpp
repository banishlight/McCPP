#pragma once
#include <TickSystem.hpp>

// Periodically evicts chunks from World's RAM caches once no player has had
// them loaded for longer than GRACE_PERIOD_SECONDS -- keeps memory bounded as
// players explore, instead of every touched chunk staying resident forever.
// A grace period + periodic sweep (rather than immediate eviction on the last
// viewer leaving) avoids repeatedly evicting/reloading a chunk a player is
// oscillating right at the view-distance edge of.
class ChunkUnloadSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;

        // How long a chunk must have had zero viewers before it's evicted.
        // Tunable -- kept as a single named constant so it's easy to adjust.
        static constexpr double GRACE_PERIOD_SECONDS = 15.0;
    private:
        static constexpr Int64 SWEEP_INTERVAL_TICKS = 100; // 5s at 20 TPS
};
