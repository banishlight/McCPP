#pragma once
#include <Standards.hpp>
#include <map>
#include <tuple>
#include <vector>
#include <mutex>

// Farmland/crops don't get resolved every game tick -- like fluids
// (FluidUpdateQueue.hpp, this is a direct structural copy of that queue),
// each position gets a scheduled recheck instead of a per-tick sweep of
// every loaded farmland/crop block. A countdown per pending position, not an
// absolute tick number, so callers on any thread (a till/plant/bone-meal
// packet handler, or CropGrowthSystem's own tick thread rescheduling a
// position) never need to know the current tick count -- only "how many
// ticks from now."
class CropGrowthQueue {
    public:
        static CropGrowthQueue& getInstance();
        // Schedules (x,y,z) to be re-resolved in delayTicks ticks. If already
        // pending with a smaller remaining countdown, the earlier one wins.
        void schedule(int x, int y, int z, int delayTicks);
        // Called once per tick (CropGrowthSystem::onTick): decrements every
        // pending countdown by 1, removing and returning positions that reach
        // zero.
        std::vector<std::tuple<int,int,int>> tick();
    private:
        CropGrowthQueue() = default;
        std::mutex _mutex;
        std::map<std::tuple<int,int,int>, int> _countdowns;
};
