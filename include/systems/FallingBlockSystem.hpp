#pragma once
#include <TickSystem.hpp>

// Simulates straight-down gravity for every tracked FallingBlockEntity each
// tick (see CheckGravityBlock, Play.cpp, for how one gets created). On
// landing, converts the entity back into a real static block via
// World::setBlock and removes the entity, broadcasting Block_Update_p +
// Remove_Entities_p. Mirrors ItemPhysicsSystem's ground-collision approach,
// simplified to vertical-only motion (nothing in this project pushes a
// falling block sideways) and always-teleport-every-tick position updates
// (falling blocks are short-lived, so there's no jitter concern worth
// throttling corrections for, unlike a tossed item's longer horizontal arc).
class FallingBlockSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
    private:
        static constexpr double GRAVITY = 0.04; // blocks/tick^2 -- matches ItemPhysicsSystem's constant
};
