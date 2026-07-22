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
// Needed symbolically by the gravity-block check (Play.cpp's CheckGravityBlock) --
// every other new block added via BlockTable's 179-row table is only ever
// referenced by name/lookup, but gravity checks need a direct ID comparison.
static constexpr Int32 SAND_BLOCK_STATE_ID = 112;
static constexpr Int32 GRAVEL_BLOCK_STATE_ID = 118;
// Level-0 (source) state IDs for the two fluids -- each fluid occupies 16
// contiguous state IDs, one per "level" property value (0-15), in the same
// order, so any level's state ID is just base + level. See FluidBlocks.hpp.
static constexpr Int32 WATER_BLOCK_STATE_ID = 80;
static constexpr Int32 LAVA_BLOCK_STATE_ID = 96;