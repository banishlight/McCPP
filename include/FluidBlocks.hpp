#pragma once
#include <Standards.hpp>

// Water/lava block-state "level" encoding -- confirmed against a real vanilla
// data report (server.jar --reports' blocks.json) and a version-pinned
// minecraft.wiki revision (not the live page, since this kind of detail can
// drift between versions), not guessed. Each fluid occupies 16 contiguous
// block-state IDs, one per "level" property value 0-15, in the same relative
// order, so level = stateId - <fluid's level-0 state ID> (see BlockIds.hpp).
//
// Level 0 is the source: permanent, never changes on its own.
// Levels 1-7 are flowing, and equal the distance-equivalent cost from the
// source -- level 1 is adjacent to a source (tallest flowing water), level 7
// is about to disappear (shallowest), i.e. level = distance. The fluid's
// "amount" (its surface height in eighths of a block) runs the other way:
// amount = 8 - level.
// Level 8 is falling: a vertical column with open space below it, fed
// continuously from above, which looks/acts full regardless of how far it is
// horizontally from the originating source.
// Levels 9-15 exist in the block-state format but vanilla's own spread logic
// never produces them naturally (the wiki notes they're only reachable via
// commands), so this project never produces them either.
//
// Water and lava do NOT decay at the same rate, not guessed: water's
// distance-equivalent (the "8 - level" cost) increases by 1 per block
// spread away from a source, but lava's increases by 2 per block, capping
// its effective spread at 3 blocks instead of water's 7. A first version of
// this project treated both fluids identically (distance = plain hop count),
// which meant lava was assigned level values real lava never produces
// (levels 1,3,5,7 -- only even levels 2,4,6 are reachable with a decrease-
// per-block of 2). NOTE: this was originally (wrongly) believed to be the
// cause of lava rendering with water's texture client-side -- it wasn't (see
// the real root cause below); this fix stands on its own merits (matching
// real lava's spread behavior) but didn't fix that symptom.
namespace Fluid {
    enum class Type { None, Water, Lava };

    Type typeOf(Int32 blockStateId);
    bool isSource(Int32 blockStateId);
    bool isFalling(Int32 blockStateId);
    // Surface height in eighths of a block (8 - level): 7 adjacent to a
    // source, 1 at the far edge. Only meaningful for flowing (non-source,
    // non-falling) blocks; returns -1 otherwise.
    int amountOf(Int32 blockStateId);
    // How much the distance-equivalent cost increases per block of
    // horizontal spread away from a source: 1 for water, 2 for lava
    // (Overworld rate -- this project has no Nether to need a different one).
    int levelDecreasePerBlock(Type type);

    Int32 sourceId(Type type);
    Int32 fallingId(Type type);
    // distance: 1 (strongest, adjacent to a source) .. 7 (weakest, about to vanish).
    Int32 flowingId(Type type, int distance);
}

// The separate "minecraft:fluid" registry (distinct numbering from
// block-state IDs above -- this is the registry the client's #minecraft:water
// / #minecraft:lava tags reference). Real protocol IDs, confirmed via a fresh
// server.jar --reports run (registries.json's minecraft:fluid entries) --
// fixed/stable, since vanilla has no datapack mechanism to add custom fluids.
static constexpr Int32 FLUID_REGISTRY_EMPTY_ID = 0;
static constexpr Int32 FLUID_REGISTRY_FLOWING_WATER_ID = 1;
static constexpr Int32 FLUID_REGISTRY_WATER_ID = 2;
static constexpr Int32 FLUID_REGISTRY_FLOWING_LAVA_ID = 3;
static constexpr Int32 FLUID_REGISTRY_LAVA_ID = 4;
