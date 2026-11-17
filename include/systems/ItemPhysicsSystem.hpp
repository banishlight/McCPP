#pragma once
#include <TickSystem.hpp>

// Advances every tracked item entity one tick (the movement itself lives in
// ItemPhysics::step), keeping ItemEntityManager's stored position authoritative
// (TryPickupNearbyItems' distance check and the despawn sweep both depend on
// it being current, not just the position an item was spawned at), then
// sends viewers whatever EntityTracker decides -- the same corrections a
// vanilla server would send, so the client's own simulation of the item
// stays in step with ours.
class ItemPhysicsSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
};
