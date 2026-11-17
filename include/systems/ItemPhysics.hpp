#pragma once
#include <Standards.hpp>

// Per-tick movement of a single dropped item entity, kept free of the World
// singleton, the entity manager and networking so it can be exercised
// directly in tests. ItemPhysicsSystem owns the world access and broadcasting
// and calls step() once per item per tick.
//
// The real client simulates every item locally between server corrections,
// so this needs to match vanilla's per-tick rules closely -- any difference
// shows up as the client's copy drifting and then snapping back.
namespace ItemPhysics {

    // Constants match vanilla. Several are single-precision in vanilla and
    // widened to double when applied; they're kept that way here so both
    // sides round identically.
    static constexpr double GRAVITY = 0.04;           // blocks/tick^2
    static constexpr double AIR_FRICTION = 0.98f;     // per-tick velocity multiplier (horizontal)
    static constexpr double VERTICAL_FRICTION = 0.98; // per-tick vertical multiplier, always
    static constexpr double HALF_WIDTH = 0.125;       // hitbox is 0.25 x 0.25 x 0.25
    static constexpr double HEIGHT = 0.25;

    // Read-only block lookup in world coordinates. Returns false when the
    // block's chunk isn't loaded; y is always within the world's build range.
    class BlockSource {
        public:
            virtual ~BlockSource() = default;
            virtual bool getBlock(int x, int y, int z, Int32& outBlockStateId) const = 0;
    };

    // Position is the bottom-center of the hitbox.
    struct State {
        double x, y, z;
        double vx, vy, vz;
    };

    struct StepResult {
        State next;
        bool onGround;
        bool inFluid;
        // Clients should get an update this tick rather than waiting for the
        // routine one: set while in a fluid, or when the velocity jumped
        // sharply (e.g. landing), matching when vanilla does it.
        bool hasImpulse;
    };

    // Advances one item by one tick. Returns false (and leaves out untouched)
    // when the item's own chunk isn't loaded, in which case it shouldn't move.
    bool step(const BlockSource& blocks, const State& current, StepResult& out);
}
