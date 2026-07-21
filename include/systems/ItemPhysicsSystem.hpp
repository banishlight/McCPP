#pragma once
#include <TickSystem.hpp>

// Simulates gravity + single-column ground collision for every tracked item
// entity each tick, keeping ItemEntityManager's stored position authoritative
// (TryPickupNearbyItems' distance check and the despawn sweep both depend on
// it being current, not just the position an item was spawned at). Position
// corrections (Teleport_Entity_p + Set_Entity_Velocity_p) are only broadcast
// when velocity changes meaningfully (see onTick) -- the client runs its own
// local physics from the last velocity it was told; correcting every tick
// fights that prediction and looks jittery.
// Constants below approximate vanilla item-entity physics for plausible-looking
// motion -- a gameplay-feel choice, not a wire-format detail, so not
// decompile-verified the way packet layouts are.
class ItemPhysicsSystem : public TickSystem {
    public:
        void onTick(Int64 tickCount) override;
        string getName() const override;
    private:
        static constexpr double GRAVITY = 0.04; // blocks/tick^2
        static constexpr double DRAG = 0.98; // horizontal velocity retained per tick
};
