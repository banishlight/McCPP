#include <tests/TestFramework.hpp>
#include <systems/ItemPhysics.hpp>
#include <BlockIds.hpp>
#include <FluidBlocks.hpp>
#include <CropBlocks.hpp>
#include <algorithm>
#include <map>
#include <tuple>

namespace {
    // In-memory world: every block is air unless set, and every chunk counts
    // as loaded unless `loaded` is cleared.
    class TestBlocks : public ItemPhysics::BlockSource {
        public:
            bool loaded = true;

            void set(int x, int y, int z, Int32 blockStateId) {
                _blocks[{x, y, z}] = blockStateId;
            }

            bool getBlock(int x, int y, int z, Int32& out) const override {
                if (!loaded) return false;
                auto it = _blocks.find({x, y, z});
                out = it == _blocks.end() ? AIR_BLOCK_STATE_ID : it->second;
                return true;
            }

        private:
            std::map<std::tuple<int, int, int>, Int32> _blocks;
    };

    constexpr int FLOOR_Y = 63;
    constexpr int SURFACE_Y = 64;

    // A one-wide channel along +x at SURFACE_Y, stone floor and side walls,
    // a source at x=0 then flowing cells stepping down by the fluid's
    // per-block decay. With wallX >= 0, a stone wall closes the channel there.
    void BuildChannel(TestBlocks& blocks, Fluid::Type type, int length, int wallX = -1) {
        for (int x = -1; x <= length + 1; x++) {
            for (int z = -1; z <= 1; z++) blocks.set(x, FLOOR_Y, z, STONE_BLOCK_STATE_ID);
            blocks.set(x, SURFACE_Y, -1, STONE_BLOCK_STATE_ID);
            blocks.set(x, SURFACE_Y, 1, STONE_BLOCK_STATE_ID);
        }
        blocks.set(-1, SURFACE_Y, 0, STONE_BLOCK_STATE_ID);
        blocks.set(0, SURFACE_Y, 0, Fluid::sourceId(type));
        int step = Fluid::levelDecreasePerBlock(type);
        for (int x = 1; x < length && x * step <= 7; x++) {
            blocks.set(x, SURFACE_Y, 0, Fluid::flowingId(type, x * step));
        }
        if (wallX >= 0) {
            for (int x = wallX; x <= length + 1; x++) blocks.set(x, SURFACE_Y, 0, STONE_BLOCK_STATE_ID);
        }
    }

    // A walled pool of still water `depth` blocks deep, floor at FLOOR_Y.
    void BuildPool(TestBlocks& blocks, int depth) {
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                blocks.set(x, FLOOR_Y, z, STONE_BLOCK_STATE_ID);
                bool edge = x == -2 || x == 2 || z == -2 || z == 2;
                for (int y = SURFACE_Y; y < SURFACE_Y + depth; y++) {
                    blocks.set(x, y, z, edge ? STONE_BLOCK_STATE_ID : Fluid::sourceId(Fluid::Type::Water));
                }
            }
        }
    }

    ItemPhysics::StepResult Step(const TestBlocks& blocks, const ItemPhysics::State& state) {
        ItemPhysics::StepResult result{};
        CHECK(ItemPhysics::step(blocks, state, result));
        return result;
    }

    // Vanilla applies these as single-precision values widened to double.
    const double WATER_DRAG = 0.99f;
    const double LAVA_DRAG = 0.95f;
    const double FRICTION = 0.98f;
}

TEST(ItemPhysicsUnloadedChunkLeavesItemAlone) {
    TestBlocks blocks;
    blocks.loaded = false;
    ItemPhysics::StepResult result{};
    CHECK(!ItemPhysics::step(blocks, {0.5, 70.0, 0.5, 0.1, -0.2, 0.0}, result));
}

TEST(ItemPhysicsRestingItemStaysPut) {
    TestBlocks blocks;
    blocks.set(0, FLOOR_Y, 0, STONE_BLOCK_STATE_ID);
    ItemPhysics::State resting{0.5, SURFACE_Y, 0.5, 0.0, 0.0, 0.0};
    ItemPhysics::StepResult result = Step(blocks, resting);
    CHECK(result.onGround);
    CHECK(!result.inFluid);
    CHECK_EQ(result.next.x, resting.x);
    CHECK_EQ(result.next.y, resting.y);
    CHECK_EQ(result.next.z, resting.z);
    CHECK_EQ(result.next.vx, 0.0);
    CHECK_EQ(result.next.vy, 0.0);
    CHECK_EQ(result.next.vz, 0.0);
}

TEST(ItemPhysicsFreeFallMatchesVanillaRecurrence) {
    TestBlocks blocks; // nothing but air
    ItemPhysics::State state{0.5, 100.0, 0.5, 0.0, 0.0, 0.0};
    for (int tick = 0; tick < 10; tick++) {
        ItemPhysics::StepResult result = Step(blocks, state);
        CHECK(!result.onGround);
        // Gravity, then move by the new velocity, then vertical drag.
        double expectedVyBeforeDrag = state.vy - ItemPhysics::GRAVITY;
        CHECK_NEAR(result.next.y, state.y + expectedVyBeforeDrag, 1e-12);
        CHECK_NEAR(result.next.vy, expectedVyBeforeDrag * ItemPhysics::VERTICAL_FRICTION, 1e-12);
        state = result.next;
    }
}

TEST(ItemPhysicsAirborneHorizontalDrag) {
    TestBlocks blocks;
    ItemPhysics::StepResult result = Step(blocks, {0.5, 100.0, 0.5, 0.2, 0.0, -0.1});
    CHECK_NEAR(result.next.x, 0.5 + 0.2, 1e-12); // moves before drag applies
    CHECK_NEAR(result.next.vx, 0.2 * FRICTION, 1e-12);
    CHECK_NEAR(result.next.vz, -0.1 * FRICTION, 1e-12);
}

TEST(ItemPhysicsFallLandsExactlyOnBlockTop) {
    TestBlocks blocks;
    blocks.set(0, FLOOR_Y, 0, STONE_BLOCK_STATE_ID);
    ItemPhysics::State state{0.5, 70.0, 0.5, 0.0, 0.0, 0.0};
    bool landed = false;
    for (int tick = 0; tick < 200 && !landed; tick++) {
        ItemPhysics::StepResult result = Step(blocks, state);
        CHECK(result.next.y >= SURFACE_Y); // never sinks into the floor
        state = result.next;
        landed = result.onGround;
    }
    REQUIRE(landed);
    CHECK_EQ(state.y, static_cast<double>(SURFACE_Y));
    CHECK_EQ(state.vy, 0.0);
}

TEST(ItemPhysicsFallsThroughCropsOntoFarmland) {
    TestBlocks blocks;
    blocks.set(0, FLOOR_Y, 0, Crop::farmlandStateFor(0));
    blocks.set(0, SURFACE_Y, 0, Crop::stateFor(Crop::Type::Wheat, 3));
    ItemPhysics::State state{0.5, 67.0, 0.5, 0.0, 0.0, 0.0};
    for (int tick = 0; tick < 100; tick++) state = Step(blocks, state).next;
    CHECK_NEAR(state.y, FLOOR_Y + 15.0 / 16.0, 1e-12);
}

TEST(ItemPhysicsSlidesAndStopsOnGround) {
    TestBlocks blocks;
    for (int x = -1; x <= 10; x++) blocks.set(x, FLOOR_Y, 0, STONE_BLOCK_STATE_ID);
    ItemPhysics::State state{0.5, SURFACE_Y, 0.5, 0.2, 0.0, 0.0};
    ItemPhysics::StepResult first = Step(blocks, state);
    CHECK(first.onGround);
    CHECK(first.next.x > state.x);          // keeps sliding instead of stopping dead
    CHECK(first.next.vx < state.vx * 0.6);  // ground friction is much stronger than air
    for (int tick = 0; tick < 100; tick++) state = Step(blocks, state).next;
    CHECK(std::abs(state.vx) < 1e-6);
}

TEST(ItemPhysicsWaterPushesDownstream) {
    TestBlocks blocks;
    BuildChannel(blocks, Fluid::Type::Water, 6);
    ItemPhysics::StepResult result = Step(blocks, {1.5, SURFACE_Y, 0.5, 0.0, 0.0, 0.0});
    CHECK(result.inFluid);
    CHECK(result.next.x > 1.5);
    CHECK_EQ(result.next.vz, 0.0); // walled channel: no sideways flow
    // One tick from rest: push 0.014, water drag, move, friction, then the
    // second end-of-tick push of 0.014.
    CHECK_NEAR(result.next.vx, 0.014 * WATER_DRAG * FRICTION + 0.014, 1e-12);
    CHECK(result.next.vy > 0.0); // submerged, so it floats up
}

TEST(ItemPhysicsLavaPushUsesMinimumSpeedFromRest) {
    TestBlocks blocks;
    BuildChannel(blocks, Fluid::Type::Lava, 4);
    ItemPhysics::StepResult result = Step(blocks, {1.5, SURFACE_Y, 0.5, 0.0, 0.0, 0.0});
    CHECK(result.inFluid);
    // Lava's 0.00233 push is below the 0.0045 floor, so from rest it's
    // boosted to it. By the second push the item is already moving faster
    // than the floor's 0.003 cutoff, so that one stays at 0.00233.
    CHECK_NEAR(result.next.vx, 0.0045 * LAVA_DRAG * FRICTION + 0.0023333333333333335, 1e-12);
}

TEST(ItemPhysicsStillWaterDoesNotPush) {
    TestBlocks blocks;
    BuildPool(blocks, 1);
    ItemPhysics::StepResult result = Step(blocks, {0.5, SURFACE_Y + 0.5, 0.5, 0.0, 0.0, 0.0});
    CHECK(result.inFluid);
    CHECK_EQ(result.next.vx, 0.0);
    CHECK_EQ(result.next.vz, 0.0);
}

TEST(ItemPhysicsFloatsJustBelowWaterSurface) {
    // A still source's surface is 8/9 of a block up. The item rises slowly
    // until the water over its bottom edge is only 0.1 deep, takes one tick
    // of gravity, dips, and rises again -- a slow bob a few tenths of a block
    // tall that stays under the surface and well off the floor, never pinned
    // to the top of the water block.
    TestBlocks blocks;
    BuildPool(blocks, 1);
    const double surface = SURFACE_Y + 8.0 / 9.0;
    ItemPhysics::State state{0.5, SURFACE_Y + 0.2, 0.5, 0.0, 0.0, 0.0};
    double lowest = 1e9, highest = -1e9;
    for (int tick = 0; tick < 400; tick++) {
        ItemPhysics::StepResult result = Step(blocks, state);
        CHECK(result.inFluid);
        state = result.next;
        if (tick >= 300) {
            lowest = std::min(lowest, state.y);
            highest = std::max(highest, state.y);
        }
    }
    CHECK(highest < surface - 0.05);
    CHECK(lowest > SURFACE_Y + 0.3);
    CHECK(highest - lowest > 0.1); // it does bob, rather than sitting still
    CHECK(highest < static_cast<double>(SURFACE_Y + 1));
}

TEST(ItemPhysicsRisesFromBottomOfDeepWater) {
    TestBlocks blocks;
    BuildPool(blocks, 3);
    ItemPhysics::State state{0.5, SURFACE_Y, 0.5, 0.0, 0.0, 0.0};
    for (int tick = 0; tick < 100; tick++) state = Step(blocks, state).next;
    CHECK(state.y > SURFACE_Y + 1.0);
}

TEST(ItemPhysicsCurrentStopsAtWall) {
    TestBlocks blocks;
    const int wallX = 6;
    BuildChannel(blocks, Fluid::Type::Water, 10, wallX);
    ItemPhysics::State state{0.5, SURFACE_Y + 0.5, 0.5, 0.0, 0.0, 0.0};
    for (int tick = 0; tick < 200; tick++) {
        state = Step(blocks, state).next;
        CHECK(state.x + ItemPhysics::HALF_WIDTH <= wallX + 1e-9);
    }
    CHECK_NEAR(state.x, wallX - ItemPhysics::HALF_WIDTH, 0.01);
    CHECK(state.y < SURFACE_Y + 1.0); // still in the channel, not on top of the wall
}
