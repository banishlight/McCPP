#pragma once
#include <Standards.hpp>
#include <vector>

// One canonical, table-driven source of {blockStateId, itemId, name} for
// every "simple" block this project knows -- single block-state, zero
// properties (no directional/waterlogged/multi-part/axis/lit variance).
// Real IDs, sourced directly from server.jar's own data generator report
// (blocks.json for block-state IDs + registries.json's "minecraft:item"
// protocol_id for item IDs), not guessed. BlockNames and ItemBlockMapping
// both delegate their lookups here internally -- their own public API is
// unchanged, this just replaces what used to be two separately hand-maintained
// if-chains/switches (which didn't scale) with one shared table.
//
// Stairs, slabs, doors, logs, and anything else with real per-instance state
// are deliberately excluded -- each would need many state permutations
// (see docs/general-documentation.md).
struct BlockTableEntry {
    Int32 blockStateId;
    Int32 itemId;
    const char* name;
};

const std::vector<BlockTableEntry>& getBlockTable();
