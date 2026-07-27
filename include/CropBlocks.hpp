#pragma once
#include <Standards.hpp>

// Farmland "moisture" and crop "age" block-state encoding -- confirmed
// against a real vanilla data report (server.jar --reports' blocks.json),
// not guessed. Same pattern as FluidBlocks.hpp's fluid "level": each block
// family occupies a contiguous run of state IDs, one per property value, in
// the same order, so property value = stateId - <that family's base state ID>
// (see BlockIds.hpp).
//
// Farmland: moisture 0 (driest) - 7 (wettest, real vanilla's threshold for
// "grows crops faster"). Wheat/carrots/potatoes: age 0 (just planted) - 7
// (mature, ready to harvest). Beetroot is the one exception with fewer
// stages: age 0-3 only.
namespace Crop {
    enum class Type { None, Wheat, Carrots, Potatoes, Beetroot };

    Int32 baseId(Type type); // age=0 state; -1 for Type::None
    int maxAge(Type type);   // 7, except Beetroot's 3; 0 for Type::None

    bool isCrop(Int32 blockStateId);
    Type typeOf(Int32 blockStateId); // Type::None if not a crop
    int ageOf(Int32 blockStateId);   // -1 if not a crop
    Int32 stateFor(Type type, int age);

    bool isFarmland(Int32 blockStateId);
    int moistureOf(Int32 blockStateId); // -1 if not farmland
    Int32 farmlandStateFor(int moisture);
}
