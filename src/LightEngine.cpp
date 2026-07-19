#include <LightEngine.hpp>
#include <Chunk.hpp>
#include <World.hpp>
#include <vector>
#include <queue>
#include <algorithm>
#include <memory>

namespace {
    constexpr int BUF_SIZE = 48; // 3x3 chunks
    constexpr int BUF_HEIGHT = Chunk::WORLD_HEIGHT;
    constexpr int MAX_LIGHT = 15;

    struct QueueEntry {
        int x, y, z; // buffer-local coordinates
        int level;
        bool sky; // true = sky channel, false = block channel
    };

    size_t bufIndex(int bx, int by, int bz) {
        return (static_cast<size_t>(by) * BUF_SIZE + bz) * BUF_SIZE + bx;
    }
}

bool LightEngine::isOpaque(Int32 blockStateId) {
    // Every block in this world's palette today (stone/dirt/grass_block) is
    // fully opaque; only air (0) is transparent. No partial-opacity blocks
    // (glass, leaves, etc.) exist yet.
    return blockStateId != 0;
}

int LightEngine::getLightEmission(Int32 blockStateId) {
    (void)blockStateId;
    return 0; // no light-emitting blocks exist yet
}

void LightEngine::computeLighting(Chunk& target, World& world) {
    int tcx = target.getChunkX();
    int tcz = target.getChunkZ();
    int baseX = tcx * 16 - 16; // buffer-local x=0 maps to this world X
    int baseZ = tcz * 16 - 16;

    // Pre-fetch the 3x3 neighborhood's terrain once (never cached itself --
    // see World::getOrGenerateTerrain).
    std::shared_ptr<Chunk> neighbors[3][3];
    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dz == 0) continue;
            neighbors[dz + 1][dx + 1] = world.getOrGenerateTerrain(tcx + dx, tcz + dz);
        }
    }

    auto blockAt = [&](int bx, int by, int bz) -> Int32 {
        if (bx < 0 || bx >= BUF_SIZE || bz < 0 || bz >= BUF_SIZE || by < 0 || by >= BUF_HEIGHT) {
            return 0;
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
        std::shared_ptr<Chunk>& n = neighbors[dz + 1][dx + 1];
        return n ? n->getBlock(localX, worldY, localZ) : 0;
    };

    std::vector<uint8_t> skyBuf(static_cast<size_t>(BUF_SIZE) * BUF_SIZE * BUF_HEIGHT, 0);
    std::vector<uint8_t> blockBuf(static_cast<size_t>(BUF_SIZE) * BUF_SIZE * BUF_HEIGHT, 0);
    std::queue<QueueEntry> queue;

    // Seed sky light: walk each column top-down, full strength until the
    // first opaque block. Gets every open-sky value correct with no BFS.
    for (int bz = 0; bz < BUF_SIZE; bz++) {
        for (int bx = 0; bx < BUF_SIZE; bx++) {
            bool blocked = false;
            for (int by = BUF_HEIGHT - 1; by >= 0 && !blocked; by--) {
                Int32 block = blockAt(bx, by, bz);
                if (isOpaque(block)) {
                    blocked = true;
                } else {
                    skyBuf[bufIndex(bx, by, bz)] = MAX_LIGHT;
                }
            }
        }
    }

    // Only enqueue seeded positions at an actual light/dark boundary (a
    // horizontal neighbor not already at max light, or the buffer edge) --
    // a perf-critical optimization, see docs/general-documentation.md,
    // "LightEngine".
    for (int bz = 0; bz < BUF_SIZE; bz++) {
        for (int bx = 0; bx < BUF_SIZE; bx++) {
            for (int by = 0; by < BUF_HEIGHT; by++) {
                if (skyBuf[bufIndex(bx, by, bz)] != MAX_LIGHT) continue;
                static const int HDX[4] = {1, -1, 0, 0};
                static const int HDZ[4] = {0, 0, 1, -1};
                bool boundary = false;
                for (int h = 0; h < 4 && !boundary; h++) {
                    int nx = bx + HDX[h], nz = bz + HDZ[h];
                    if (nx < 0 || nx >= BUF_SIZE || nz < 0 || nz >= BUF_SIZE) {
                        boundary = true; // buffer edge -- can't verify, be safe
                    } else if (skyBuf[bufIndex(nx, by, nz)] != MAX_LIGHT) {
                        boundary = true;
                    }
                }
                if (boundary) {
                    queue.push({bx, by, bz, MAX_LIGHT, true});
                }
            }
        }
    }

    // Seed block light: any position with a light-emitting block. No such
    // blocks exist yet, so this never enqueues anything -- a correct no-op,
    // not a stub, and ready for torches/glowstone/etc. once they exist.
    for (int by = 0; by < BUF_HEIGHT; by++) {
        for (int bz = 0; bz < BUF_SIZE; bz++) {
            for (int bx = 0; bx < BUF_SIZE; bx++) {
                int emission = getLightEmission(blockAt(bx, by, bz));
                if (emission > 0) {
                    blockBuf[bufIndex(bx, by, bz)] = static_cast<uint8_t>(emission);
                    queue.push({bx, by, bz, emission, false});
                }
            }
        }
    }

    static const int DX[6] = {1, -1, 0, 0, 0, 0};
    static const int DY[6] = {0, 0, 1, -1, 0, 0};
    static const int DZ[6] = {0, 0, 0, 0, 1, -1};

    while (!queue.empty()) {
        QueueEntry e = queue.front();
        queue.pop();
        for (int d = 0; d < 6; d++) {
            int nx = e.x + DX[d];
            int ny = e.y + DY[d];
            int nz = e.z + DZ[d];
            if (nx < 0 || nx >= BUF_SIZE || nz < 0 || nz >= BUF_SIZE || ny < 0 || ny >= BUF_HEIGHT) {
                continue;
            }
            Int32 destBlock = blockAt(nx, ny, nz);
            int destOpacity = isOpaque(destBlock) ? MAX_LIGHT : 0;
            int newLevel;
            // Sky light propagating straight down through a transparent block
            // doesn't attenuate, unlike every other direction.
            if (e.sky && DY[d] == -1 && e.level == MAX_LIGHT && destOpacity == 0) {
                newLevel = MAX_LIGHT;
            } else {
                newLevel = e.level - std::max(destOpacity, 1);
                if (newLevel < 0) newLevel = 0;
            }
            if (newLevel <= 0) continue;
            std::vector<uint8_t>& buf = e.sky ? skyBuf : blockBuf;
            size_t ni = bufIndex(nx, ny, nz);
            if (buf[ni] < newLevel) {
                buf[ni] = static_cast<uint8_t>(newLevel);
                queue.push({nx, ny, nz, newLevel, e.sky});
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