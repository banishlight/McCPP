#include <ItemProperties.hpp>
#include <algorithm>

namespace ItemProperties {

Int32 getMaxStackSize(Int32 itemId) {
    const std::vector<ItemTableEntry>& table = getItemTable();
    auto it = std::find_if(table.begin(), table.end(), [itemId](const ItemTableEntry& entry) {
        return itemId == entry.itemId;
    });
    return (it != table.end()) ? it->maxStackSize : 64;
}

EquipmentSlot getEquipmentSlot(Int32 itemId) {
    if (itemId < 0) return EquipmentSlot::None;
    const std::vector<ItemTableEntry>& table = getItemTable();
    auto it = std::find_if(table.begin(), table.end(), [itemId](const ItemTableEntry& entry) {
        return itemId == entry.itemId;
    });
    return (it != table.end()) ? it->equipmentSlot : EquipmentSlot::None;
}

}
