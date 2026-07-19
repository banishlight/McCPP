#pragma once
#include <Standards.hpp>

// Real block-state IDs, sourced from the vanilla data generator report
// (server.jar --reports), not guessed. Shared by the terrain generator,
// block break/place, and the item<->block mapping table -- previously
// duplicated as local constants in several files.
static constexpr Int32 AIR_BLOCK_STATE_ID = 0; // stable across versions since the 1.13 flattening
static constexpr Int32 STONE_BLOCK_STATE_ID = 1;
static constexpr Int32 DIRT_BLOCK_STATE_ID = 10;
static constexpr Int32 GRASS_BLOCK_STATE_ID = 9; // snowy=false (default)