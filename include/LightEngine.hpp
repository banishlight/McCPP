#pragma once
#include <Standards.hpp>

class Chunk;
class World;

// Sky + block light, stored per block in each Chunk.
//
// Light spreads from a level-L block to each neighbor as
// L - max(1, absorption(neighbor)), except that full-strength (15) sky
// light passes straight down through fully transparent blocks without
// dimming. Since 15 < 16, a change can only ever affect light within the
// 3x3 chunks around it (vertically it's unbounded, horizontally it isn't).
//
// Two entry points:
//  - computeLighting: lights a whole chunk from scratch (generation/load).
//  - updateForBlockChange: patches light around one changed block, for
//    edits to already-lit chunks. Both produce identical results.
class LightEngine {
    public:
        // Full lighting for `target`, reading blocks from its 3x3
        // neighborhood (missing neighbors are fetched/generated via `world`).
        static void computeLighting(Chunk& target, World& world);

        // Same, with the neighborhood supplied directly: neighbors[dz + 1][dx + 1]
        // is the chunk at (target.x + dx, target.z + dz); the center entry is
        // ignored, and a nullptr neighbor reads as all air.
        static void computeLighting(Chunk& target, const Chunk* const neighbors[3][3]);

        // Whether replacing oldBlock with newBlock can change any light at
        // all. When false, no light update is needed.
        static bool affectsLight(Int32 oldBlockStateId, Int32 newBlockStateId);

        // Updates light after the block at (worldX, worldY, worldZ) changed
        // from oldBlockStateId. The new block must already be set in its
        // chunk. region[dz + 1][dx + 1] is the chunk at (cx + dx, cz + dz)
        // around the edited chunk (cx, cz); nullptr entries are treated as
        // not loaded and left alone. Light is written in place into the
        // region's chunks, and touched[dz + 1][dx + 1] reports which ones
        // actually changed.
        static void updateForBlockChange(Chunk* const region[3][3], int worldX, int worldY, int worldZ,
                                         Int32 oldBlockStateId, bool touched[3][3]);

        // How much a block dims light passing into it: 0 (air, plants),
        // 1 (fluids), or 15 (opaque).
        static int lightAbsorption(Int32 blockStateId);
        static int lightEmission(Int32 blockStateId);
};
