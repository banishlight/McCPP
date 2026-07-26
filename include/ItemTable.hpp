#pragma once
#include <Standards.hpp>
#include <vector>

// One canonical, generated, non-guessed source of {itemId, name,
// maxStackSize, equipmentSlot, itemCategory} for every real vanilla item
// (1333 entries) -- see ItemTable.cpp's header comment for exactly where the
// data came from. Used by ItemNames (name<->id) for inventory persistence/
// display, and by ItemProperties (maxStackSize/equipmentSlot/itemCategory)
// for stack-merge math, shift-click armor routing, and (itemCategory)
// Enchantment.hpp's item-enchantment applicability. Deliberately separate
// from BlockTable/ItemBlockMapping (block placement/breaking's own
// item<->block cross-reference), which are unaffected by this table's
// existence.
enum class EquipmentSlot { None, Head, Chest, Legs, Feet, Offhand };

// Which "kind of tool/weapon" an item is, for enchantment applicability
// (Enchantment.hpp) -- e.g. Sharpness needs Sword or Axe, Fortune needs
// Pickaxe/Axe/Shovel/Hoe. Deliberately narrow (only the categories real
// enchantments actually key off of) rather than a general tool taxonomy.
enum class ItemCategory { None, Sword, Axe, Pickaxe, Shovel, Hoe, Bow, Crossbow, Trident, Mace, FishingRod };

struct ItemTableEntry {
    Int32 itemId;
    const char* name;
    Int32 maxStackSize;
    EquipmentSlot equipmentSlot;
    ItemCategory itemCategory;
};

const std::vector<ItemTableEntry>& getItemTable();
