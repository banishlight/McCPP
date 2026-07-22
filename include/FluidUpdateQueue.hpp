#pragma once
#include <Standards.hpp>
#include <map>
#include <tuple>
#include <vector>
#include <mutex>

// Fluids don't get resolved every game tick like a moving entity would --
// vanilla's own fluid ticking is scheduled (water re-checks every 5 ticks,
// lava every 30, see FluidBlocks.hpp/docs), not a per-tick sweep of every
// loaded fluid block. This queue holds a countdown per pending position
// rather than an absolute tick number, so callers on any thread (network
// threads reacting to a break/place edit, or FluidSystem's own tick thread
// scheduling a position's neighbors) never need to know the current tick
// count -- only "how many ticks from now."
class FluidUpdateQueue {
    public:
        static FluidUpdateQueue& getInstance();
        // Schedules (x,y,z) to be re-resolved in delayTicks ticks. If already
        // pending with a smaller remaining countdown, the earlier one wins --
        // scheduling never pushes an already-imminent check further out.
        void schedule(int x, int y, int z, int delayTicks);
        // Called once per tick (FluidSystem::onTick): decrements every
        // pending countdown by 1, removing and returning positions that reach
        // zero.
        std::vector<std::tuple<int,int,int>> tick();
    private:
        FluidUpdateQueue() = default;
        std::mutex _mutex;
        std::map<std::tuple<int,int,int>, int> _countdowns;
};
