#pragma once
#include <Standards.hpp>

// Decides, tick by tick, which movement packets a tracked entity's viewers
// get -- following vanilla's server-side rules, so the client's own local
// simulation of the entity is corrected exactly as often (and in the same
// way) as a vanilla client expects. Pure logic: the caller does the sending.
namespace EntityTracker {

    struct Config {
        int updateInterval;  // ticks between routine updates (items: 20)
        bool trackVelocity;  // whether velocity changes are sent (items: yes)
    };

    // Per-entity record of what viewers were last told. Start it from the
    // values the spawn packet carried.
    struct State {
        int ticks = 0;
        int updatesSinceTeleport = 0;
        double sentX = 0.0, sentY = 0.0, sentZ = 0.0;
        double sentVx = 0.0, sentVy = 0.0, sentVz = 0.0;
        bool sentOnGround = false;
    };

    enum class PositionPacket { None, Delta, Teleport };

    // What to send this tick. When both are present, send velocity first.
    struct Update {
        bool sendVelocity = false;
        double vx = 0.0, vy = 0.0, vz = 0.0;

        PositionPacket position = PositionPacket::None;
        Int16 deltaX = 0, deltaY = 0, deltaZ = 0; // for Delta
        double x = 0.0, y = 0.0, z = 0.0;         // for Teleport
        bool onGround = false;
    };

    // Call once per tick per entity, after it has moved. hasImpulse asks for
    // an update this tick instead of waiting for the next routine one (items
    // set it whenever they're in a fluid or their velocity jumped).
    Update tick(State& state, const Config& config,
                double x, double y, double z, double vx, double vy, double vz,
                bool onGround, bool hasImpulse);
}
