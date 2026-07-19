#pragma once
#include <Standards.hpp>

// Real item registry IDs, sourced from the vanilla data generator report
// (server.jar --reports, the "minecraft:item" registry), not guessed. A
// separate numeric space from block-state IDs (BlockIds.hpp) -- e.g. stone's
// item ID (1) happens to collide with STONE_BLOCK_STATE_ID (1), but dirt's
// (28) and grass_block's (27) don't match their block-state IDs (10, 9) at all.
static constexpr Int32 STONE_ITEM_ID = 1;
static constexpr Int32 DIRT_ITEM_ID = 28;
static constexpr Int32 GRASS_BLOCK_ITEM_ID = 27;

// Returns -1 (unmapped) for anything not in the small hardcoded table above --
// not a general item/block registry, just enough to cover this world's four
// block types (air never needs a mapping either direction).
Int32 itemIdToBlockStateId(Int32 itemId);
Int32 blockStateIdToItemId(Int32 blockStateId);