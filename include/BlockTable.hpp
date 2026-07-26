#pragma once
#include <Standards.hpp>
#include <vector>

// One canonical, table-driven source of {blockStateId, itemId, name} for
// every block in the real 1.21 registry (1057 entries, generated from
// server.jar --reports -- blocks.json for each block's default state id +
// registries.json's "minecraft:item" protocol_id, not guessed). BlockNames
// and ItemBlockMapping both delegate their lookups here internally -- their
// own public API is unchanged.
//
// Blocks with real properties (stairs, logs, doors, etc.) are represented by
// their single default state -- correct for round-tripping as long as this
// project never places a *different* state of that block. The two fluids
// (water/lava) are the deliberate exception: never rows here, because they
// need a numeric "level" property this table can't express -- ChunkNbtCodec
// and BlockNames both special-case them directly instead. grass_block's
// snowy=true state is the other known gap (BlockNames.cpp's own special
// case) -- this project never generates it, so it's untested, not fixed.
//
// If a future feature needs more than one state of an existing-property
// block, that needs a real per-block-instance property system -- not another
// scattered special case. See docs/internal-documentation.md.
struct BlockTableEntry {
    Int32 blockStateId;
    Int32 itemId;
    const char* name;
};

const std::vector<BlockTableEntry>& getBlockTable();
