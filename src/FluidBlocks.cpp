#include <FluidBlocks.hpp>
#include <BlockIds.hpp>

namespace {
    Int32 baseId(Fluid::Type type) {
        switch (type) {
            case Fluid::Type::Water: return WATER_BLOCK_STATE_ID;
            case Fluid::Type::Lava: return LAVA_BLOCK_STATE_ID;
            default: return -1;
        }
    }
}

namespace Fluid {

Type typeOf(Int32 blockStateId) {
    if (blockStateId >= WATER_BLOCK_STATE_ID && blockStateId <= WATER_BLOCK_STATE_ID + 15) return Type::Water;
    if (blockStateId >= LAVA_BLOCK_STATE_ID && blockStateId <= LAVA_BLOCK_STATE_ID + 15) return Type::Lava;
    return Type::None;
}

bool isSource(Int32 blockStateId) {
    Type type = typeOf(blockStateId);
    if (type == Type::None) return false;
    return blockStateId == baseId(type);
}

bool isFalling(Int32 blockStateId) {
    Type type = typeOf(blockStateId);
    if (type == Type::None) return false;
    return blockStateId == baseId(type) + 8;
}

int distanceOf(Int32 blockStateId) {
    Type type = typeOf(blockStateId);
    if (type == Type::None) return -1;
    int level = blockStateId - baseId(type);
    if (level < 1 || level > 7) return -1; // source, falling, or an unreachable 9-15 level
    return 8 - level;
}

int levelDecreasePerBlock(Type type) {
    return (type == Type::Lava) ? 2 : 1;
}

Int32 sourceId(Type type) { return baseId(type); }
Int32 fallingId(Type type) { return baseId(type) + 8; }
// Int32 flowingId(Type type, int distance) { return baseId(type) + (8 - distance); }
Int32 flowingId(Type type, int distance) { return baseId(type) + distance; }

}
