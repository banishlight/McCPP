#pragma once
#include <Standards.hpp>

// Overrides for blocks whose break-drop differs from the default (drop
// itself, already handled by ItemBlockMapping::blockStateIdToItemId). A
// block with no entry here just falls back to that default -- this table
// only needs an entry when a block drops something ELSE (stone -> cobblestone,
// cobweb -> string, both 100% chance) or drops that something else only some
// of the time (oak_leaves -> oak_sapling, short_grass -> wheat_seeds).
//
// Chances here are each block's real, base (no-tool-applied) drop chance --
// sourced directly from server.jar's own bundled loot tables
// (data/minecraft/loot_table/blocks/*.json), not guessed. No Silk Touch or
// Fortune support yet (this project has no enchantment system at all) -- see
// the checklist comment in BlockDropTable.cpp for what that will need to
// change here once those tools exist.
namespace BlockDropTable {
    // Returns false if blockStateId has no override entry at all (caller
    // should fall back to the default ItemBlockMapping-based drop). Returns
    // true if an entry exists -- *outItemId is then either the real item to
    // drop (this call's chance roll succeeded) or -1 (roll failed, meaning
    // this break should drop nothing at all, not fall back to the default).
    bool TryResolveDrop(Int32 blockStateId, Int32& outItemId);
}
