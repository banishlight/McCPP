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
// Needed symbolically by fluid "mixing" reactions (Play.cpp's
// TryApplyFluidReaction) for the same reason sand/gravel are above -- a
// direct ID comparison, not a name lookup.
static constexpr Int32 COBBLESTONE_BLOCK_STATE_ID = 14;
static constexpr Int32 OBSIDIAN_BLOCK_STATE_ID = 2354;
// Needed symbolically by the biome-aware surface layering in
// NoiseChunkGenerator, same reason as the other direct-comparison constants
// above.
static constexpr Int32 SNOW_BLOCK_STATE_ID = 5781;
// oak_log/oak_leaves both have real properties (axis; distance/persistent/
// waterlogged) this project doesn't model as a general mechanism -- same
// treatment as GRASS_BLOCK_STATE_ID's "snowy=false" comment above, one fixed
// state picked directly rather than exposing the property space. axis=y is
// the vertical trunk orientation (also this block's own default state); the
// leaves property values only affect a decay mechanic this project doesn't
// implement, so any valid state looks identical -- using the block's default.
static constexpr Int32 OAK_LOG_STATE_ID = 131; // axis=y (default)
static constexpr Int32 OAK_LEAVES_STATE_ID = 264; // distance=7, persistent=false, waterlogged=false (default)
// Ground plants placed by NoiseChunkGenerator, same direct-ID-use reasoning
// as the terrain blocks above (also ordinary BlockTable rows, so breaking
// one still drops a real item, unlike the log/leaves pair above).
static constexpr Int32 SHORT_GRASS_STATE_ID = 2005;
static constexpr Int32 POPPY_STATE_ID = 2077;
static constexpr Int32 DANDELION_STATE_ID = 2075;