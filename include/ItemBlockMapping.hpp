#pragma once
#include <Standards.hpp>

// Real item registry IDs, sourced from the vanilla data generator report
// (server.jar --reports, the "minecraft:item" registry), not guessed. A
// separate numeric space from block-state IDs (BlockIds.hpp) -- e.g. stone's
// item ID (1) happens to collide with STONE_BLOCK_STATE_ID (1), but dirt's
// (28) and grass_block's (27) don't match their block-state IDs (10, 9) at all.
// These three are also ordinary BlockTable rows (see BlockTable.cpp's
// original 3 entries) -- named here only for readability at their one use site.
static constexpr Int32 STONE_ITEM_ID = 1;
static constexpr Int32 DIRT_ITEM_ID = 28;
static constexpr Int32 GRASS_BLOCK_ITEM_ID = 27;
// oak_log is NOT a BlockTable row (it has a real axis property BlockTable's
// single-state design excludes -- see OAK_LOG_STATE_ID in BlockIds.hpp), so
// it needs its own special case in both functions below rather than going
// through the table scan every other block uses.
static constexpr Int32 OAK_LOG_ITEM_ID = 132;

// Backed by BlockTable's full table scan (182+ entries) for every ordinary
// block, plus the oak_log special case above. Returns -1 (unmapped) for
// anything not covered by either -- e.g. oak_leaves, which deliberately has
// no item mapping (real vanilla leaves only rarely drop a sapling/stick, a
// chance-based mechanic not implemented here).
Int32 itemIdToBlockStateId(Int32 itemId);
Int32 blockStateIdToItemId(Int32 blockStateId);