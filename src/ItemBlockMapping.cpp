#include <ItemBlockMapping.hpp>
#include <BlockIds.hpp>
#include <BlockTable.hpp>
#include <algorithm>

Int32 itemIdToBlockStateId(Int32 itemId) {
    if (itemId < 0) return -1; // -1 is BlockTable's own "no item" sentinel, never a real item id
    const std::vector<BlockTableEntry>& table = getBlockTable();
    auto it = std::find_if(table.begin(), table.end(), [itemId](const BlockTableEntry& entry) {
        return itemId == entry.itemId;
    });
    return (it != table.end()) ? it->blockStateId : -1;
}

Int32 blockStateIdToItemId(Int32 blockStateId) {
    const std::vector<BlockTableEntry>& table = getBlockTable();
    auto it = std::find_if(table.begin(), table.end(), [blockStateId](const BlockTableEntry& entry) {
        return blockStateId == entry.blockStateId;
    });
    return (it != table.end()) ? it->itemId : -1;
}