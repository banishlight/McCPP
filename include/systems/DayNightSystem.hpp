#pragma once
#include <TickSystem.hpp>

// Advances World's day/night clock every tick and periodically broadcasts it
// to every connected player via Update_Time_p, driving client-side sky/sun
// rendering. No gamerule system exists in this project (see LevelData's own
// "game rules out of scope" stance), so there's no doDaylightCycle-style
// freeze -- time always advances continuously, matching vanilla's default.
class DayNightSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
    private:
        static constexpr Int64 BROADCAST_INTERVAL_TICKS = 20; // 1 second -- clients interpolate smoothly between updates
};
