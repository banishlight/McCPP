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

// Backed entirely by BlockTable's full table scan (1057 entries, one row per
// real vanilla block). Returns -1 (unmapped) for anything BlockTable itself
// marks as itemId -1 (a block with no matching item in the real registry --
// wall signs, technical blocks, etc.) or any id not in the table at all.
Int32 itemIdToBlockStateId(Int32 itemId);
Int32 blockStateIdToItemId(Int32 blockStateId);