#pragma once
#include <Standards.hpp>
#include <vector>

// One canonical, generated, non-guessed source of {itemId, name,
// maxStackSize, equipmentSlot} for every real vanilla item (1333 entries) --
// see ItemTable.cpp's header comment for exactly where the data came from.
// Used by ItemNames (name<->id) for inventory persistence/display, and by
// ItemProperties (maxStackSize/equipmentSlot) for stack-merge math and
// shift-click armor routing. Deliberately separate from BlockTable/
// ItemBlockMapping (block placement/breaking's own item<->block
// cross-reference), which are unaffected by this table's existence.
enum class EquipmentSlot { None, Head, Chest, Legs, Feet, Offhand };

struct ItemTableEntry {
    Int32 itemId;
    const char* name;
    Int32 maxStackSize;
    EquipmentSlot equipmentSlot;
};

const std::vector<ItemTableEntry>& getItemTable();
