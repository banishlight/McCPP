#pragma once
#include <TickSystem.hpp>

// Periodically persists every chunk edited/generated since the last save
// (World::takeDirtyChunksSnapshot) plus level.dat, matching vanilla's default
// autosave cadence.
class AutosaveSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;

        // Saves every dirty chunk + level.dat immediately, regardless of tick
        // count -- shared by onTick's periodic cadence and the shutdown save
        // hook in main().
        static void saveNow();
    private:
        static constexpr Int64 AUTOSAVE_INTERVAL_TICKS = 5 * 60 * 20; // 5 minutes at 20 TPS, matches vanilla's default
};
