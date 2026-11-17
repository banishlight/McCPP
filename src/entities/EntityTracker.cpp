#include <entities/EntityTracker.hpp>
#include <network/PacketUtils.hpp>

namespace EntityTracker {

namespace {
    // Vanilla's thresholds.
    constexpr double MIN_POSITION_CHANGE_SQ = 7.62939453125e-6; // ~0.0028 blocks
    constexpr double MIN_VELOCITY_CHANGE_SQ = 1.0e-7;
    constexpr int FORCED_POSITION_INTERVAL = 60;  // ticks; resend position even if unchanged
    constexpr int MAX_UPDATES_BETWEEN_TELEPORTS = 400;
}

Update tick(State& state, const Config& config,
            double x, double y, double z, double vx, double vy, double vz,
            bool onGround, bool hasImpulse) {
    Update update;
    const int ticks = state.ticks++;
    if (ticks % config.updateInterval != 0 && !hasImpulse) return update;

    state.updatesSinceTeleport++;

    const double dx = x - state.sentX, dy = y - state.sentY, dz = z - state.sentZ;
    const bool moved = dx * dx + dy * dy + dz * dz >= MIN_POSITION_CHANGE_SQ;
    const bool sendPosition = moved || ticks % FORCED_POSITION_INTERVAL == 0;

    Int16 deltaX = 0, deltaY = 0, deltaZ = 0;
    const bool fitsDelta = encodePositionDelta(state.sentX, x, deltaX)
                        && encodePositionDelta(state.sentY, y, deltaY)
                        && encodePositionDelta(state.sentZ, z, deltaZ);

    if (!fitsDelta || state.updatesSinceTeleport > MAX_UPDATES_BETWEEN_TELEPORTS || state.sentOnGround != onGround) {
        update.position = PositionPacket::Teleport;
        update.x = x; update.y = y; update.z = z;
        state.sentOnGround = onGround;
        state.updatesSinceTeleport = 0;
    } else if (sendPosition) {
        update.position = PositionPacket::Delta;
        update.deltaX = deltaX; update.deltaY = deltaY; update.deltaZ = deltaZ;
    }
    update.onGround = onGround;

    if ((config.trackVelocity || hasImpulse) && ticks > 0) {
        const double dvx = vx - state.sentVx, dvy = vy - state.sentVy, dvz = vz - state.sentVz;
        const double changeSq = dvx * dvx + dvy * dvy + dvz * dvz;
        const bool stopped = vx == 0.0 && vy == 0.0 && vz == 0.0;
        if (changeSq > MIN_VELOCITY_CHANGE_SQ || (changeSq > 0.0 && stopped)) {
            update.sendVelocity = true;
            update.vx = vx; update.vy = vy; update.vz = vz;
            state.sentVx = vx; state.sentVy = vy; state.sentVz = vz;
        }
    }

    if (update.position != PositionPacket::None) {
        state.sentX = x; state.sentY = y; state.sentZ = z;
    }
    return update;
}

}
