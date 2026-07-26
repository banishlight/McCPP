#pragma once
#include <Standards.hpp>
#include <ItemTable.hpp>

// Per-item facts backed by ItemTable -- real max stack size and (for the
// handful of items that have one) which equipment slot an item belongs in.
namespace ItemProperties {
    // Falls back to 64 (vanilla's most common stack size) for any itemId not
    // found in ItemTable -- e.g. -1 (empty slot). Real items always resolve
    // to their true value (1, 16, or 64), never guessed.
    Int32 getMaxStackSize(Int32 itemId);

    // EquipmentSlot::None for -1 (empty slot), any unrecognized itemId, or
    // any real item that isn't armor/elytra/shield.
    EquipmentSlot getEquipmentSlot(Int32 itemId);
}
