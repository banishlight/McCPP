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
// (data/minecraft/loot_table/blocks/*.json), not guessed.
namespace BlockDropTable {
    // The breaking tool's enchantment-relevant properties. Scaffolding only,
    // for now: this project can't yet receive/store enchantment data on an
    // item at all (Set_Creative_Mode_Slot_p explicitly refuses any item
    // carrying data components -- see PacketUtils.hpp's unpackSlot), so
    // every real caller constructs this as ToolInfo{} (the defaults) until
    // that groundwork lands. CheckDrop below is already correct for that
    // case, and needs no other changes once fortuneLevel/silkTouch can
    // actually be populated from a real held item.
    struct ToolInfo {
        int fortuneLevel = 0;
        bool silkTouch = false;
    };

    // Resolves what a block should drop when broken with the given tool.
    // Returns false if blockStateId has no override entry AND silkTouch is
    // false -- caller should fall back to the default ItemBlockMapping-based
    // drop (drop itself). Returns true otherwise, with *outItemId set to
    // either the real item to drop, or -1 (an override entry's chance roll
    // failed -- drop nothing at all, do NOT fall back to the default).
    bool CheckDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId);
}
