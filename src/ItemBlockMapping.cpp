#include <ItemBlockMapping.hpp>
#include <BlockIds.hpp>

Int32 itemIdToBlockStateId(Int32 itemId) {
    switch (itemId) {
        case STONE_ITEM_ID: return STONE_BLOCK_STATE_ID;
        case DIRT_ITEM_ID: return DIRT_BLOCK_STATE_ID;
        case GRASS_BLOCK_ITEM_ID: return GRASS_BLOCK_STATE_ID;
        default: return -1;
    }
}

Int32 blockStateIdToItemId(Int32 blockStateId) {
    switch (blockStateId) {
        case STONE_BLOCK_STATE_ID: return STONE_ITEM_ID;
        case DIRT_BLOCK_STATE_ID: return DIRT_ITEM_ID;
        case GRASS_BLOCK_STATE_ID: return GRASS_BLOCK_ITEM_ID;
        default: return -1;
    }
}