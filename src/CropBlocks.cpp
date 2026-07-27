#include <CropBlocks.hpp>
#include <BlockIds.hpp>
#include <algorithm>

namespace Crop {

Int32 baseId(Type type) {
    switch (type) {
        case Type::Wheat: return WHEAT_BASE_STATE_ID;
        case Type::Carrots: return CARROTS_BASE_STATE_ID;
        case Type::Potatoes: return POTATOES_BASE_STATE_ID;
        case Type::Beetroot: return BEETROOT_BASE_STATE_ID;
        default: return -1;
    }
}

int maxAge(Type type) {
    if (type == Type::None) return 0;
    return (type == Type::Beetroot) ? 3 : 7;
}

Type typeOf(Int32 blockStateId) {
    for (Type type : {Type::Wheat, Type::Carrots, Type::Potatoes, Type::Beetroot}) {
        Int32 base = baseId(type);
        if (blockStateId >= base && blockStateId <= base + maxAge(type)) return type;
    }
    return Type::None;
}

bool isCrop(Int32 blockStateId) {
    return typeOf(blockStateId) != Type::None;
}

int ageOf(Int32 blockStateId) {
    Type type = typeOf(blockStateId);
    if (type == Type::None) return -1;
    return blockStateId - baseId(type);
}

Int32 stateFor(Type type, int age) {
    return baseId(type) + std::clamp(age, 0, maxAge(type));
}

bool isFarmland(Int32 blockStateId) {
    return blockStateId >= FARMLAND_BLOCK_STATE_ID && blockStateId <= FARMLAND_BLOCK_STATE_ID + 7;
}

int moistureOf(Int32 blockStateId) {
    if (!isFarmland(blockStateId)) return -1;
    return blockStateId - FARMLAND_BLOCK_STATE_ID;
}

Int32 farmlandStateFor(int moisture) {
    return FARMLAND_BLOCK_STATE_ID + std::clamp(moisture, 0, 7);
}

}
