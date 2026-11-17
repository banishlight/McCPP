#include <tests/TestFramework.hpp>
#include <entities/EntityTracker.hpp>

using EntityTracker::PositionPacket;

namespace {
    const EntityTracker::Config ITEM{20, true};

    EntityTracker::State StartAt(double x, double y, double z) {
        EntityTracker::State state;
        state.sentX = x; state.sentY = y; state.sentZ = z;
        return state;
    }
}

TEST(TrackerSendsNothingBetweenRoutineUpdatesWithoutImpulse) {
    EntityTracker::State state = StartAt(0, 64, 0);
    EntityTracker::tick(state, ITEM, 0, 64, 0, 0, 0, 0, false, false); // tick 0: routine slot
    for (int tick = 1; tick < 20; tick++) {
        EntityTracker::Update update = EntityTracker::tick(state, ITEM, 0.01 * tick, 64, 0, 0.01, 0, 0, false, false);
        CHECK(update.position == PositionPacket::None);
        CHECK(!update.sendVelocity);
    }
    EntityTracker::Update routine = EntityTracker::tick(state, ITEM, 0.2, 64, 0, 0.01, 0, 0, false, false);
    CHECK(routine.position == PositionPacket::Delta);
    CHECK_EQ(routine.deltaX, static_cast<Int16>(0.2 * 4096 + 0.5));
    CHECK(routine.sendVelocity);
}

TEST(TrackerSendsEveryTickWithImpulse) {
    EntityTracker::State state = StartAt(0, 64, 0);
    double x = 0.0, vx = 0.0;
    for (int tick = 0; tick < 10; tick++) {
        vx += 0.01;
        x += vx;
        EntityTracker::Update update = EntityTracker::tick(state, ITEM, x, 64, 0, vx, 0, 0, false, true);
        CHECK(update.position == PositionPacket::Delta);
        if (tick > 0) CHECK(update.sendVelocity); // never on the very first tick
    }
    CHECK_EQ(state.sentX, x);
}

TEST(TrackerSkipsTinyMovesButResendsPositionEvery60Ticks) {
    EntityTracker::State state = StartAt(0, 64, 0);
    for (int tick = 0; tick < 60; tick++) {
        EntityTracker::Update update = EntityTracker::tick(state, ITEM, 0.001, 64, 0, 0, 0, 0, false, true);
        if (tick == 0) CHECK(update.position == PositionPacket::Delta); // tick 0 is a forced slot
        else CHECK(update.position == PositionPacket::None);
    }
    EntityTracker::Update forced = EntityTracker::tick(state, ITEM, 0.001, 64, 0, 0, 0, 0, false, true);
    CHECK(forced.position == PositionPacket::Delta);
}

TEST(TrackerTeleportsWhenOnGroundChanges) {
    EntityTracker::State state = StartAt(0, 64, 0);
    EntityTracker::Update landed = EntityTracker::tick(state, ITEM, 0, 64, 0, 0, 0, 0, true, true);
    CHECK(landed.position == PositionPacket::Teleport);
    CHECK(landed.onGround);
    EntityTracker::Update still = EntityTracker::tick(state, ITEM, 0.1, 64, 0, 0, 0, 0, true, true);
    CHECK(still.position == PositionPacket::Delta);
}

TEST(TrackerTeleportsWhenMoveTooLargeForDelta) {
    EntityTracker::State state = StartAt(0, 64, 0);
    EntityTracker::Update update = EntityTracker::tick(state, ITEM, 20, 64, 0, 0, 0, 0, false, true);
    CHECK(update.position == PositionPacket::Teleport);
    CHECK_EQ(update.x, 20.0);
}

TEST(TrackerSendsVelocityWhenItStops) {
    EntityTracker::State state = StartAt(0, 64, 0);
    state.sentVx = 1.0e-4; // smaller than the change threshold
    EntityTracker::tick(state, ITEM, 0, 64, 0, 1.0e-4, 0, 0, false, true);
    EntityTracker::Update stopped = EntityTracker::tick(state, ITEM, 0, 64, 0, 0, 0, 0, false, true);
    CHECK(stopped.sendVelocity);
    CHECK_EQ(stopped.vx, 0.0);
}
