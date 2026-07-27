#pragma once
#include <TickSystem.hpp>

// Drains CropGrowthQueue every tick and re-derives each due position via
// ResolveCropGrowth (Play.cpp) -- see CropBlocks.hpp for the block-state
// encoding and CropGrowthQueue.hpp for why this is a scheduled queue rather
// than a per-tick sweep of every loaded farmland/crop block.
class CropGrowthSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
};
