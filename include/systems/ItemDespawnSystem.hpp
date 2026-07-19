#pragma once
#include <TickSystem.hpp>

// Sweeps ItemEntityManager every tick for drops older than DESPAWN_SECONDS
// (wall-clock, not tick count -- items are spawned from network threads, not
// the tick thread) and removes them, broadcasting Remove_Entities_p to
// whoever has that item's chunk loaded. Matches vanilla's 5-minute despawn.
class ItemDespawnSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
    private:
        static constexpr double DESPAWN_SECONDS = 300.0;
};
