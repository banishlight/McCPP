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
// Levels 1-7 are flowing, and are backwards from "distance from source" --
// distance 1 (adjacent to a source) is level 7, distance 7 (about to
// disappear) is level 1, i.e. level = 8 - distance.
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
// per-block of 2) -- the client has no real precedent for those states on an
// actual lava block and appears to fall back to a water-like rendering for
// them, which is what this was actually causing, not a wire-encoding bug.
namespace Fluid {
    enum class Type { None, Water, Lava };

    Type typeOf(Int32 blockStateId);
    bool isSource(Int32 blockStateId);
    bool isFalling(Int32 blockStateId);
    // Only meaningful for flowing (non-source, non-falling) blocks; returns
    // -1 otherwise.
    int distanceOf(Int32 blockStateId);
    // How much the distance-equivalent cost increases per block of
    // horizontal spread away from a source: 1 for water, 2 for lava
    // (Overworld rate -- this project has no Nether to need a different one).
    int levelDecreasePerBlock(Type type);

    Int32 sourceId(Type type);
    Int32 fallingId(Type type);
    // distance: 1 (strongest, adjacent to a source) .. 7 (weakest, about to vanish).
    Int32 flowingId(Type type, int distance);
}
