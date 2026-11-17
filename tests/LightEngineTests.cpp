#include <tests/TestFramework.hpp>
#include <LightEngine.hpp>
#include <Chunk.hpp>
#include <BlockIds.hpp>
#include <FluidBlocks.hpp>
#include <CropBlocks.hpp>
#include <memory>
#include <sstream>
#include <vector>

namespace {
    const Int32 WATER = Fluid::sourceId(Fluid::Type::Water);
    const Int32 STONE = STONE_BLOCK_STATE_ID;
    const Int32 AIR = AIR_BLOCK_STATE_ID;

    // A 5x5 grid of chunks (chunk coordinates -2..2). Edits go in the center
    // chunk (0, 0); a light update can reach the inner 3x3, and the outer ring
    // supplies the blocks those chunks' full lighting reads.
    class TestWorld {
        public:
            TestWorld() {
                for (int cz = -2; cz <= 2; cz++) {
                    for (int cx = -2; cx <= 2; cx++) {
                        _chunks.push_back(std::make_unique<Chunk>(cx, cz));
                    }
                }
                // Uneven ground with a roofed-over area and a water pool, so
                // light has shadows, caves and partial absorption to handle.
                for (int x = -32; x < 48; x++) {
                    for (int z = -32; z < 48; z++) {
                        int h = GroundHeight(x, z);
                        for (int y = 40; y <= h; y++) set(x, y, z, STONE);
                        if (x >= -6 && x < 6 && z >= 2 && z < 12) set(x, h + 5, z, STONE);
                        if (x >= 9 && x < 20 && z >= 4 && z < 13) {
                            for (int y = h - 3; y <= h; y++) set(x, y, z, WATER);
                        }
                    }
                }
            }

            static int GroundHeight(int x, int z) { return 60 + ((x * 7 + z * 13) % 5 + 5) % 5; }

            Chunk* chunk(int cx, int cz) {
                if (cx < -2 || cx > 2 || cz < -2 || cz > 2) return nullptr;
                return _chunks[(cz + 2) * 5 + (cx + 2)].get();
            }

            void set(int x, int y, int z, Int32 id) {
                int cx = floorDiv16(x), cz = floorDiv16(z);
                chunk(cx, cz)->setBlock(x - cx * 16, y, z - cz * 16, id);
            }

            Int32 get(int x, int y, int z) {
                int cx = floorDiv16(x), cz = floorDiv16(z);
                return chunk(cx, cz)->getBlock(x - cx * 16, y, z - cz * 16);
            }

            int sky(int x, int y, int z) {
                int cx = floorDiv16(x), cz = floorDiv16(z);
                return chunk(cx, cz)->getSkyLight(x - cx * 16, y, z - cz * 16);
            }

            void neighborsOf(int cx, int cz, const Chunk* out[3][3]) {
                for (int dz = -1; dz <= 1; dz++)
                    for (int dx = -1; dx <= 1; dx++)
                        out[dz + 1][dx + 1] = chunk(cx + dx, cz + dz);
            }

            // Full lighting for the inner 3x3, into the chunks themselves.
            void lightFully() {
                for (int cz = -1; cz <= 1; cz++) {
                    for (int cx = -1; cx <= 1; cx++) {
                        const Chunk* n[3][3];
                        neighborsOf(cx, cz, n);
                        LightEngine::computeLighting(*chunk(cx, cz), n);
                    }
                }
            }

            // Changes a block in the center chunk and patches light incrementally.
            void edit(int x, int y, int z, Int32 id) {
                Int32 old = get(x, y, z);
                set(x, y, z, id);
                Chunk* region[3][3];
                for (int dz = -1; dz <= 1; dz++)
                    for (int dx = -1; dx <= 1; dx++)
                        region[dz + 1][dx + 1] = chunk(dx, dz);
                bool touched[3][3];
                LightEngine::updateForBlockChange(region, x, y, z, old, touched);
            }

            // Recomputes the inner 3x3 from scratch on copies and counts
            // positions whose light differs from what's stored now.
            int countMismatches(std::string& firstMismatch) {
                int mismatches = 0;
                for (int cz = -1; cz <= 1; cz++) {
                    for (int cx = -1; cx <= 1; cx++) {
                        Chunk fresh(*chunk(cx, cz));
                        const Chunk* n[3][3];
                        neighborsOf(cx, cz, n);
                        LightEngine::computeLighting(fresh, n);
                        const Chunk& stored = *chunk(cx, cz);
                        for (int y = Chunk::WORLD_MIN_Y; y < Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT; y++) {
                            for (int lz = 0; lz < 16; lz++) {
                                for (int lx = 0; lx < 16; lx++) {
                                    int es = fresh.getSkyLight(lx, y, lz), as = stored.getSkyLight(lx, y, lz);
                                    int eb = fresh.getBlockLight(lx, y, lz), ab = stored.getBlockLight(lx, y, lz);
                                    if (es == as && eb == ab) continue;
                                    if (mismatches++ == 0) {
                                        std::ostringstream m;
                                        m << "at (" << cx * 16 + lx << ", " << y << ", " << cz * 16 + lz << "): sky "
                                          << as << " expected " << es << ", block " << ab << " expected " << eb;
                                        firstMismatch = m.str();
                                    }
                                }
                            }
                        }
                    }
                }
                return mismatches;
            }

        private:
            std::vector<std::unique_ptr<Chunk>> _chunks;
    };

    void CheckMatchesFullRecompute(TestWorld& world) {
        std::string first;
        int mismatches = world.countMismatches(first);
        if (mismatches != 0) TestFramework::reportFailure(__FILE__, __LINE__, std::to_string(mismatches) + " mismatched positions, first " + first);
    }

    // Small deterministic generator so failures are reproducible.
    struct Lcg {
        UInt32 state;
        int next(int bound) {
            state = state * 1664525u + 1013904223u;
            return static_cast<int>((state >> 8) % static_cast<UInt32>(bound));
        }
    };
}

TEST(LightAffectsLightOnlyWhenAbsorptionOrEmissionChanges) {
    CHECK(LightEngine::affectsLight(AIR, STONE));
    CHECK(LightEngine::affectsLight(AIR, WATER));
    CHECK(!LightEngine::affectsLight(AIR, Crop::stateFor(Crop::Type::Wheat, 2)));
    CHECK(!LightEngine::affectsLight(STONE, COBBLESTONE_BLOCK_STATE_ID));
    CHECK(!LightEngine::affectsLight(WATER, Fluid::flowingId(Fluid::Type::Water, 3)));
}

TEST(LightSkyDimsOneLevelPerBlockOfWater) {
    TestWorld world;
    world.lightFully();
    // A three-deep water column in a stone-walled shaft on open ground, so
    // the only light reaching it comes straight down from the sky.
    const int x = 2, z = 14;
    const int h = TestWorld::GroundHeight(x, z);
    for (int i = 1; i <= 3; i++) {
        world.edit(x + 1, h + i, z, STONE);
        world.edit(x - 1, h + i, z, STONE);
        world.edit(x, h + i, z + 1, STONE);
        world.edit(x, h + i, z - 1, STONE);
    }
    for (int i = 1; i <= 3; i++) world.edit(x, h + i, z, WATER);
    CHECK_EQ(world.sky(x, h + 4, z), 15);
    CHECK_EQ(world.sky(x, h + 3, z), 14);
    CHECK_EQ(world.sky(x, h + 2, z), 13);
    CHECK_EQ(world.sky(x, h + 1, z), 12);
    CheckMatchesFullRecompute(world);
}

TEST(LightPlacingBlockCastsShadowAndRemovingRestoresIt) {
    TestWorld world;
    world.lightFully();
    // A block floating well above the ground, open on every side.
    const int x = 8, z = 0, y = 80;
    CHECK_EQ(world.sky(x, y - 1, z), 15);
    world.edit(x, y, z, STONE);
    CHECK_EQ(world.sky(x, y, z), 0);
    CHECK_EQ(world.sky(x, y - 1, z), 14); // lit from the sides now
    CheckMatchesFullRecompute(world);
    world.edit(x, y, z, AIR);
    CHECK_EQ(world.sky(x, y - 1, z), 15);
    CheckMatchesFullRecompute(world);
}

TEST(LightOpeningRoofLetsSkyIn) {
    TestWorld world;
    world.lightFully();
    const int x = 0, z = 6; // under the roof
    const int h = TestWorld::GroundHeight(x, z);
    int before = world.sky(x, h + 1, z);
    CHECK(before < 15);
    world.edit(x, h + 5, z, AIR);
    CHECK_EQ(world.sky(x, h + 1, z), 15);
    CheckMatchesFullRecompute(world);
}

TEST(LightEditsAlongChunkEdgesUpdateNeighbors) {
    TestWorld world;
    world.lightFully();
    for (int i = 0; i < 16; i += 3) {
        world.edit(0, 70, i, STONE);
        world.edit(15, 70, i, STONE);
        world.edit(i, 70, 0, STONE);
        world.edit(i, 70, 15, STONE);
    }
    CheckMatchesFullRecompute(world);
    world.edit(0, 70, 0, AIR);
    world.edit(15, 70, 15, WATER);
    CheckMatchesFullRecompute(world);
}

TEST(LightRandomEditsMatchFullRecompute) {
    TestWorld world;
    world.lightFully();
    Lcg rng{12345};
    const Int32 choices[3] = {AIR, STONE, WATER};
    for (int i = 0; i < 400; i++) {
        int x = rng.next(16), z = rng.next(16);
        int y = TestWorld::GroundHeight(x, z) - 4 + rng.next(14);
        world.edit(x, y, z, choices[rng.next(3)]);
    }
    CheckMatchesFullRecompute(world);
}
