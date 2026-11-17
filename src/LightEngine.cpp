#include <LightEngine.hpp>
#include <Chunk.hpp>
#include <World.hpp>
#include <BlockIds.hpp>
#include <CropBlocks.hpp>
#include <FluidBlocks.hpp>
#include <vector>
#include <algorithm>
#include <memory>

namespace {
    constexpr int BUF_SIZE = 48; // 3x3 chunks
    constexpr int BUF_HEIGHT = Chunk::WORLD_HEIGHT;
    constexpr int MAX_LIGHT = 15;
    constexpr int TOP_Y = Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1;

    constexpr int DX[6] = {1, -1, 0, 0, 0, 0};
    constexpr int DY[6] = {0, 0, 1, -1, 0, 0};
    constexpr int DZ[6] = {0, 0, 0, 0, 1, -1};
    constexpr int DOWN = 3; // index of (0, -1, 0) above

    // Light reaching a block with the given absorption from a neighbor at
    // `level`. Full-strength sky light keeps going straight down through
    // fully transparent blocks; everything else loses at least one level.
    int Propagate(int level, bool sky, bool downward, int absorption) {
        if (sky && downward && level == MAX_LIGHT && absorption == 0) return MAX_LIGHT;
        int next = level - std::max(absorption, 1);
        return next > 0 ? next : 0;
    }

    // Sky light a top-of-world block gets from the open sky above it.
    int SkyFromAbove(int absorption) {
        return Propagate(MAX_LIGHT, true, true, absorption);
    }

    struct QueueEntry {
        int x, y, z; // buffer-local coordinates
        int level;
        bool sky; // true = sky channel, false = block channel
    };

    size_t bufIndex(int bx, int by, int bz) {
        return (static_cast<size_t>(by) * BUF_SIZE + bz) * BUF_SIZE + bx;
    }
}

int LightEngine::lightAbsorption(Int32 blockStateId) {
    if (blockStateId == AIR_BLOCK_STATE_ID) return 0;
    if (blockStateId == SHORT_GRASS_STATE_ID || blockStateId == POPPY_STATE_ID || blockStateId == DANDELION_STATE_ID) return 0;
    if (Crop::isCrop(blockStateId)) return 0;
    // Non-solid blocks that don't let sky light straight through (fluids)
    // dim light by one level per block, as in vanilla.
    if (Fluid::typeOf(blockStateId) != Fluid::Type::None) return 1;
    return MAX_LIGHT;
}

int LightEngine::lightEmission(Int32 blockStateId) {
    (void)blockStateId;
    return 0; // no light-emitting blocks exist yet
}

bool LightEngine::affectsLight(Int32 oldBlockStateId, Int32 newBlockStateId) {
    return lightAbsorption(oldBlockStateId) != lightAbsorption(newBlockStateId)
        || lightEmission(oldBlockStateId) != lightEmission(newBlockStateId);
}

// ---- Full computation ----

void LightEngine::computeLighting(Chunk& target, World& world) {
    int tcx = target.getChunkX();
    int tcz = target.getChunkZ();
    // Pre-fetch the 3x3 neighborhood's terrain once (never cached itself --
    // see World::getOrGenerateTerrain).
    std::shared_ptr<Chunk> owned[3][3];
    const Chunk* neighbors[3][3] = {};
    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dz == 0) continue;
            owned[dz + 1][dx + 1] = world.getOrGenerateTerrain(tcx + dx, tcz + dz);
            neighbors[dz + 1][dx + 1] = owned[dz + 1][dx + 1].get();
        }
    }
    computeLighting(target, neighbors);
}

void LightEngine::computeLighting(Chunk& target, const Chunk* const neighbors[3][3]) {
    int tcx = target.getChunkX();
    int tcz = target.getChunkZ();
    int baseX = tcx * 16 - 16; // buffer-local x=0 maps to this world X
    int baseZ = tcz * 16 - 16;

    auto blockAt = [&](int bx, int by, int bz) -> Int32 {
        if (bx < 0 || bx >= BUF_SIZE || bz < 0 || bz >= BUF_SIZE || by < 0 || by >= BUF_HEIGHT) {
            return AIR_BLOCK_STATE_ID;
        }
        int worldX = baseX + bx;
        int worldZ = baseZ + bz;
        int worldY = Chunk::WORLD_MIN_Y + by;
        int cx = floorDiv16(worldX);
        int cz = floorDiv16(worldZ);
        int localX = worldX - cx * 16;
        int localZ = worldZ - cz * 16;
        int dx = cx - tcx, dz = cz - tcz;
        if (dx == 0 && dz == 0) return target.getBlock(localX, worldY, localZ);
        // dx, dz are guaranteed in [-1, 1] here since bx/bz are bounded to
        // BUF_SIZE (48 = exactly 3 chunks centered on the target).
        const Chunk* n = neighbors[dz + 1][dx + 1];
        return n ? n->getBlock(localX, worldY, localZ) : AIR_BLOCK_STATE_ID;
    };

    std::vector<uint8_t> skyBuf(static_cast<size_t>(BUF_SIZE) * BUF_SIZE * BUF_HEIGHT, 0);
    std::vector<uint8_t> blockBuf(static_cast<size_t>(BUF_SIZE) * BUF_SIZE * BUF_HEIGHT, 0);
    // BFS frontier, one entry per (position, channel) pending propagation.
    // A plain std::queue (std::deque-backed) was measured -- via a real
    // gameplay profile -- allocating a new block on essentially every push,
    // 87% of every allocation call in the entire server. Backed by a
    // thread_local vector instead, so each thread keeps its own buffer, safe
    // with no locking. clear() drops the logical size but keeps the
    // underlying capacity, so a thread's buffer grows to whatever its
    // largest chunk ever needed and then never allocates again -- the FIFO
    // read side is a plain index (queueHead) instead of popping from the
    // front, since nothing here needs the popped entries' storage back.
    thread_local std::vector<QueueEntry> queueStorage;
    queueStorage.clear();
    size_t queueHead = 0;

    // Seed sky light: walk each column top-down, full strength until the
    // first block that absorbs any light. Gets every open-sky value correct
    // with no BFS. A partly-absorbing block at the very top gets its own
    // (dimmed) value from the sky here, since there's no block above it to
    // spread from.
    for (int bz = 0; bz < BUF_SIZE; bz++) {
        for (int bx = 0; bx < BUF_SIZE; bx++) {
            for (int by = BUF_HEIGHT - 1; by >= 0; by--) {
                int absorption = lightAbsorption(blockAt(bx, by, bz));
                if (absorption == 0) {
                    skyBuf[bufIndex(bx, by, bz)] = MAX_LIGHT;
                    continue;
                }
                if (by == BUF_HEIGHT - 1) {
                    int level = SkyFromAbove(absorption);
                    if (level > 0) {
                        skyBuf[bufIndex(bx, by, bz)] = static_cast<uint8_t>(level);
                        queueStorage.push_back({bx, by, bz, level, true});
                    }
                }
                break;
            }
        }
    }

    // Only enqueue seeded positions at an actual light/dark boundary -- a
    // horizontal neighbor not already at max light, the buffer edge, or a
    // partly-absorbing block directly below (sky light continues down into
    // it, dimmed). Enqueuing every seeded position instead was a measured,
    // perf-critical cost.
    for (int bz = 0; bz < BUF_SIZE; bz++) {
        for (int bx = 0; bx < BUF_SIZE; bx++) {
            for (int by = 0; by < BUF_HEIGHT; by++) {
                if (skyBuf[bufIndex(bx, by, bz)] != MAX_LIGHT) continue;
                static const int HDX[4] = {1, -1, 0, 0};
                static const int HDZ[4] = {0, 0, 1, -1};
                bool boundary = by > 0 && skyBuf[bufIndex(bx, by - 1, bz)] != MAX_LIGHT;
                for (int h = 0; h < 4 && !boundary; h++) {
                    int nx = bx + HDX[h], nz = bz + HDZ[h];
                    if (nx < 0 || nx >= BUF_SIZE || nz < 0 || nz >= BUF_SIZE) {
                        boundary = true; // buffer edge -- can't verify, be safe
                    } else if (skyBuf[bufIndex(nx, by, nz)] != MAX_LIGHT) {
                        boundary = true;
                    }
                }
                if (boundary) {
                    queueStorage.push_back({bx, by, bz, MAX_LIGHT, true});
                }
            }
        }
    }

    // Seed block light from every light-emitting block.
    for (int by = 0; by < BUF_HEIGHT; by++) {
        for (int bz = 0; bz < BUF_SIZE; bz++) {
            for (int bx = 0; bx < BUF_SIZE; bx++) {
                int emission = lightEmission(blockAt(bx, by, bz));
                if (emission > 0) {
                    blockBuf[bufIndex(bx, by, bz)] = static_cast<uint8_t>(emission);
                    queueStorage.push_back({bx, by, bz, emission, false});
                }
            }
        }
    }

    while (queueHead < queueStorage.size()) {
        QueueEntry e = queueStorage[queueHead++];
        for (int d = 0; d < 6; d++) {
            int nx = e.x + DX[d];
            int ny = e.y + DY[d];
            int nz = e.z + DZ[d];
            if (nx < 0 || nx >= BUF_SIZE || nz < 0 || nz >= BUF_SIZE || ny < 0 || ny >= BUF_HEIGHT) {
                continue;
            }
            int newLevel = Propagate(e.level, e.sky, d == DOWN, lightAbsorption(blockAt(nx, ny, nz)));
            if (newLevel <= 0) continue;
            std::vector<uint8_t>& buf = e.sky ? skyBuf : blockBuf;
            size_t ni = bufIndex(nx, ny, nz);
            if (buf[ni] < newLevel) {
                buf[ni] = static_cast<uint8_t>(newLevel);
                queueStorage.push_back({nx, ny, nz, newLevel, e.sky});
            }
        }
    }

    // Write back only the center chunk's own 16x16 columns.
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int bx = 16 + x;
            int bz = 16 + z;
            for (int y = 0; y < BUF_HEIGHT; y++) {
                int worldY = Chunk::WORLD_MIN_Y + y;
                size_t bi = bufIndex(bx, y, bz);
                target.setSkyLight(x, worldY, z, skyBuf[bi]);
                target.setBlockLight(x, worldY, z, blockBuf[bi]);
            }
        }
    }
}

// ---- Incremental update ----

namespace {
    // World-coordinate access to the light and blocks of a 3x3-chunk region,
    // recording which chunks get written.
    class Region {
        public:
            Region(Chunk* const chunks[3][3], int centerChunkX, int centerChunkZ, bool touched[3][3])
                : _cx(centerChunkX), _cz(centerChunkZ), _touched(touched) {
                for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) _chunks[i][j] = chunks[i][j];
            }

            // False for positions outside the build height or in a chunk
            // that isn't part of the region.
            bool contains(int x, int y, int z) const {
                if (y < Chunk::WORLD_MIN_Y || y > TOP_Y) return false;
                int s = slot(x, z);
                return s >= 0 && _chunks[s / 3][s % 3];
            }

            // The accessors below require contains(x, y, z).
            Int32 block(int x, int y, int z) const {
                return chunkAt(x, z)->getBlock(local(x), y, local(z));
            }

            int light(bool sky, int x, int y, int z) const {
                const Chunk* c = chunkAt(x, z);
                return sky ? c->getSkyLight(local(x), y, local(z)) : c->getBlockLight(local(x), y, local(z));
            }

            void setLight(bool sky, int x, int y, int z, int level) {
                int s = slot(x, z);
                Chunk* c = _chunks[s / 3][s % 3];
                if (sky) c->setSkyLight(local(x), y, local(z), level);
                else c->setBlockLight(local(x), y, local(z), level);
                _touched[s / 3][s % 3] = true;
            }

        private:
            static int local(int v) { return v - floorDiv16(v) * 16; }
            // Index (row * 3 + column) of the chunk holding (x, z), or -1
            // when it's outside the region.
            int slot(int x, int z) const {
                int dx = floorDiv16(x) - _cx, dz = floorDiv16(z) - _cz;
                if (dx < -1 || dx > 1 || dz < -1 || dz > 1) return -1;
                return (dz + 1) * 3 + (dx + 1);
            }
            Chunk* chunkAt(int x, int z) const {
                int s = slot(x, z);
                return _chunks[s / 3][s % 3];
            }

            Chunk* _chunks[3][3];
            int _cx, _cz;
            bool (*_touched)[3];
    };

    struct Pos { int x, y, z; int level; };

    // Light a block provides on its own, regardless of neighbors: emission
    // for block light, open sky for a top-of-world block's sky light.
    int IntrinsicLight(const Region& region, bool sky, int x, int y, int z) {
        Int32 id = region.block(x, y, z);
        if (!sky) return LightEngine::lightEmission(id);
        return y == TOP_Y ? SkyFromAbove(LightEngine::lightAbsorption(id)) : 0;
    }

    // Standard two-pass relight of one channel after the block at (x, y, z)
    // changed: first clear every block whose light may have depended on the
    // old value (flood outward while levels keep decreasing), collecting the
    // brighter blocks bordering the cleared area; then spread light back in
    // from those and from any light sources inside it.
    void RelightChannel(Region& region, bool sky, int x, int y, int z) {
        thread_local std::vector<Pos> removal;
        thread_local std::vector<Pos> increase;
        removal.clear();
        increase.clear();

        removal.push_back({x, y, z, region.light(sky, x, y, z)});
        region.setLight(sky, x, y, z, 0);

        for (size_t head = 0; head < removal.size(); head++) {
            const Pos q = removal[head];
            for (int d = 0; d < 6; d++) {
                int nx = q.x + DX[d], ny = q.y + DY[d], nz = q.z + DZ[d];
                if (!region.contains(nx, ny, nz)) continue;
                int neighborLevel = region.light(sky, nx, ny, nz);
                if (neighborLevel == 0) continue;
                // Dimmer than q, or full sky light directly below full sky
                // light: possibly lit through q, so clear it too. Otherwise
                // it has its own source and will relight the area.
                bool dependent = neighborLevel < q.level
                              || (sky && d == DOWN && q.level == MAX_LIGHT && neighborLevel == MAX_LIGHT);
                if (dependent) {
                    region.setLight(sky, nx, ny, nz, 0);
                    removal.push_back({nx, ny, nz, neighborLevel});
                } else {
                    increase.push_back({nx, ny, nz, 0});
                }
            }
            int intrinsic = IntrinsicLight(region, sky, q.x, q.y, q.z);
            if (intrinsic > 0) {
                region.setLight(sky, q.x, q.y, q.z, intrinsic);
                increase.push_back({q.x, q.y, q.z, 0});
            }
        }

        for (size_t head = 0; head < increase.size(); head++) {
            const Pos q = increase[head];
            int level = region.light(sky, q.x, q.y, q.z);
            if (level == 0) continue;
            for (int d = 0; d < 6; d++) {
                int nx = q.x + DX[d], ny = q.y + DY[d], nz = q.z + DZ[d];
                if (!region.contains(nx, ny, nz)) continue;
                int next = Propagate(level, sky, d == DOWN, LightEngine::lightAbsorption(region.block(nx, ny, nz)));
                if (next > region.light(sky, nx, ny, nz)) {
                    region.setLight(sky, nx, ny, nz, next);
                    increase.push_back({nx, ny, nz, 0});
                }
            }
        }
    }
}

void LightEngine::updateForBlockChange(Chunk* const region[3][3], int worldX, int worldY, int worldZ,
                                       Int32 oldBlockStateId, bool touched[3][3]) {
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) touched[i][j] = false;
    if (!region[1][1]) return;
    Int32 newBlock = region[1][1]->getBlock(worldX - floorDiv16(worldX) * 16, worldY, worldZ - floorDiv16(worldZ) * 16);
    if (!affectsLight(oldBlockStateId, newBlock)) return;

    Region view(region, floorDiv16(worldX), floorDiv16(worldZ), touched);
    if (!view.contains(worldX, worldY, worldZ)) return;
    RelightChannel(view, true, worldX, worldY, worldZ);
    RelightChannel(view, false, worldX, worldY, worldZ);
}
