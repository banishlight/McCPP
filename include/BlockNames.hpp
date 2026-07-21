#pragma once
#include <Standards.hpp>

// Name<->block-state-ID translation for on-disk chunk NBT: vanilla never
// stores raw numeric block-state IDs on disk (those are a network/runtime-only,
// registry-order-dependent concept) -- only {Name, Properties} compounds.
// Only 4 block-state IDs exist in this project today (see BlockIds.hpp), so
// this is a small hardcoded table, same style as ItemBlockMapping.hpp.
namespace BlockNames {
    // Unknown names (overwhelmingly likely on a real vanilla import, given
    // how small this project's block palette still is) substitute
    // STONE_BLOCK_STATE_ID -- solid/safe, no fall-through hazards. Each
    // distinct unknown name is logged once (not once per block instance) so
    // import fidelity loss is visible rather than silent. `minecraft:air`/an
    // empty name map to air specifically so empty space doesn't turn solid.
    // Only grass_block's `snowy` property is stateful today; a snowy=true
    // grass block isn't modeled, so it falls back like any other unknown state.
    Int32 blockNameToStateId(const string& name, bool snowy = false);

    // Reverse direction, for chunk encode. Every ID this project can actually
    // produce is one of the 4 known ones.
    string blockStateIdToName(Int32 blockStateId);
}
