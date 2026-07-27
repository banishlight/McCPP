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
    // drop (drop itself, count 1). Returns true otherwise, with *outItemId
    // set to either the real item to drop, or -1 (an override entry's chance
    // roll failed -- drop nothing at all, do NOT fall back to the default).
    // *outCount is only meaningful when *outItemId >= 0 -- a real count
    // (possibly a random real vanilla range, e.g. carrots/potatoes' 1-4).
    bool CheckDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId, Int32& outCount);

    // A handful of blocks drop a SEPARATE, additional item on top of
    // CheckDrop's result -- real vanilla's mature wheat/beetroot loot tables
    // have a second loot pool rolling bonus wheat_seeds/beetroot_seeds
    // alongside the primary wheat/beetroot drop, sourced directly from
    // data/minecraft/loot_table/blocks/{wheat,beetroots}.json's own
    // "apply_bonus"/"binomial_with_bonus_count" function (extra=3,
    // probability=4/7), not guessed or approximated as a flat range. Returns
    // false if blockStateId has no bonus entry at all. Returns true
    // otherwise with *outItemId set and *outCount possibly 0 (the binomial
    // roll can legitimately produce zero bonus items -- caller should just
    // not spawn anything in that case, not treat 0 as "no entry").
    bool CheckBonusDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId, Int32& outCount);
}
