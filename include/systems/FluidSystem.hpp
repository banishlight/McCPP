#pragma once
#include <TickSystem.hpp>

// Drains FluidUpdateQueue every tick and re-derives each due position via
// ResolveFluid (Play.cpp) -- see FluidBlocks.hpp/docs for the block-state
// encoding and FluidUpdateQueue.hpp for why this is a scheduled queue rather
// than a per-tick sweep of every loaded fluid block.
class FluidSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
};
