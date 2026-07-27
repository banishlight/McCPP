#include <BlockDropTable.hpp>
#include <BlockIds.hpp>
#include <CropBlocks.hpp>
#include <ItemBlockMapping.hpp>
#include <vector>
#include <random>

namespace {
    // Real item protocol IDs, sourced from server.jar --reports'
    // registries.json (minecraft:item), not guessed. Not put in
    // ItemBlockMapping.hpp's shared constants since nothing outside this
    // table needs them by name.
    constexpr Int32 COBBLESTONE_ITEM_ID = 35;
    constexpr Int32 STRING_ITEM_ID = 850;
    constexpr Int32 OAK_SAPLING_ITEM_ID = 48;
    constexpr Int32 WHEAT_SEEDS_ITEM_ID = 853;
    constexpr Int32 WHEAT_ITEM_ID = 854;
    constexpr Int32 CARROT_ITEM_ID = 1097;
    constexpr Int32 POTATO_ITEM_ID = 1098;
    constexpr Int32 BEETROOT_ITEM_ID = 1154;
    constexpr Int32 BEETROOT_SEEDS_ITEM_ID = 1155;

    struct DropTableEntry {
        Int32 blockStateId;
        Int32 itemId;
        double chance; // 0.0-1.0, base chance with no tool applied
        Int32 minCount = 1;
        Int32 maxCount = 1; // uniform random in [minCount, maxCount] when the chance succeeds
    };

    // *** CHECKLIST for adding real Fortune scaling later ***
    // Every entry here is currently a block's real loot table collapsed down
    // to its base (fortuneLevel=0) chance. CheckDrop already takes a
    // ToolInfo::fortuneLevel, but nothing here uses it yet -- the real
    // per-level values are already sitting in the loot table json this was
    // sourced from (e.g. oak_leaves' sapling chance is
    // [0.05, 0.0625, 0.083333336, 0.1] for fortune 0-3); re-extract from
    // data/minecraft/loot_table/blocks/*.json rather than guess when this is
    // built, the same way these base values were. Silk Touch itself is
    // already handled generically in CheckDrop below (it makes ANY block
    // drop itself, no per-entry value needed) -- NOT modeled: real vanilla
    // grass_block drops dirt normally and only drops itself under Silk
    // Touch, i.e. it needs an ordinary 100%-chance entry here (dirt) for the
    // non-Silk-Touch case, same shape as stone/cobweb above; not added since
    // nobody asked for it yet, just flagged as the next obvious candidate.
    // *** ADD NEW ENTRIES HERE, ONE PER LINE. ***
    const std::vector<DropTableEntry>& getDropTable() {
        static const std::vector<DropTableEntry> table = {
            {STONE_BLOCK_STATE_ID, COBBLESTONE_ITEM_ID, 1.0},
            {COBWEB_BLOCK_STATE_ID, STRING_ITEM_ID, 1.0},
            {OAK_LEAVES_STATE_ID, OAK_SAPLING_ITEM_ID, 0.05},
            {SHORT_GRASS_STATE_ID, WHEAT_SEEDS_ITEM_ID, 0.125},
            // Crops: breaking an immature plant (any age below its real max)
            // always drops exactly 1 of its seed/planting item; a mature
            // plant drops its harvest item instead. Wheat/beetroot ALSO drop
            // a separate bonus-seed stack when mature -- see getBonusDropTable
            // below, a second, independent roll (this project's break-drop
            // path can spawn more than one item stack per break; see
            // Player_Action_p in Play.cpp for where both get combined).
            {Crop::stateFor(Crop::Type::Wheat, 0), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 1), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 2), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 3), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 4), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 5), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 6), WHEAT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Wheat, 7), WHEAT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 0), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 1), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 2), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 3), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 4), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 5), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 6), CARROT_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Carrots, 7), CARROT_ITEM_ID, 1.0, 1, 4},
            {Crop::stateFor(Crop::Type::Potatoes, 0), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 1), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 2), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 3), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 4), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 5), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 6), POTATO_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Potatoes, 7), POTATO_ITEM_ID, 1.0, 1, 4},
            {Crop::stateFor(Crop::Type::Beetroot, 0), BEETROOT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Beetroot, 1), BEETROOT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Beetroot, 2), BEETROOT_SEEDS_ITEM_ID, 1.0},
            {Crop::stateFor(Crop::Type::Beetroot, 3), BEETROOT_ITEM_ID, 1.0},
        };
        return table;
    }

    // thread_local: break handling can run concurrently across multiple
    // connections, and std::mt19937 isn't itself thread-safe to share.
    double rollChance() {
        thread_local std::mt19937 generator(std::random_device{}());
        thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator);
    }

    Int32 rollCount(Int32 minCount, Int32 maxCount) {
        if (maxCount <= minCount) return minCount;
        thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<Int32> distribution(minCount, maxCount);
        return distribution(generator);
    }

    struct BonusDropTableEntry {
        Int32 blockStateId;
        Int32 itemId;
        int trials;         // real vanilla's "extra" -- fortuneLevel adds to this
        double probability; // per-trial success chance
    };

    // Wheat/beetroot's mature-only bonus seed drop -- a genuinely separate
    // loot pool in real vanilla (data/minecraft/loot_table/blocks/
    // {wheat,beetroots}.json's "apply_bonus"/"binomial_with_bonus_count"
    // function), not a guessed flat range: 3 independent trials at
    // probability 4/7 each (0.5714286), summed -- can legitimately land
    // anywhere from 0 to 3, weighted toward the middle (mean ~1.71), not a
    // flat "always 2-3". fortuneLevel (currently always 0, see ToolInfo's own
    // comment) adds directly to the trial count, matching the real formula
    // (n = extra + fortune level) with zero extra work once Fortune is real.
    const std::vector<BonusDropTableEntry>& getBonusDropTable() {
        static const std::vector<BonusDropTableEntry> table = {
            {Crop::stateFor(Crop::Type::Wheat, 7), WHEAT_SEEDS_ITEM_ID, 3, 0.5714286},
            {Crop::stateFor(Crop::Type::Beetroot, 3), BEETROOT_SEEDS_ITEM_ID, 3, 0.5714286},
        };
        return table;
    }

    Int32 rollBinomial(int trials, double probability) {
        int successes = 0;
        for (int i = 0; i < trials; i++) {
            if (rollChance() < probability) successes++;
        }
        return successes;
    }
}

namespace BlockDropTable {

bool CheckDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId, Int32& outCount) {
    if (tool.silkTouch) {
        // Silk Touch bypasses the table entirely -- every block drops
        // itself, including ones with no entry above at all.
        outItemId = blockStateIdToItemId(blockStateId);
        outCount = 1;
        return true;
    }
    for (const DropTableEntry& entry : getDropTable()) {
        if (entry.blockStateId == blockStateId) {
            // tool.fortuneLevel is intentionally unused for now -- see the
            // CHECKLIST comment above getDropTable().
            if (rollChance() < entry.chance) {
                outItemId = entry.itemId;
                outCount = rollCount(entry.minCount, entry.maxCount);
            } else {
                outItemId = -1;
                outCount = 0;
            }
            return true;
        }
    }
    return false;
}

bool CheckBonusDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId, Int32& outCount) {
    for (const BonusDropTableEntry& entry : getBonusDropTable()) {
        if (entry.blockStateId == blockStateId) {
            outItemId = entry.itemId;
            outCount = rollBinomial(entry.trials + tool.fortuneLevel, entry.probability);
            return true;
        }
    }
    return false;
}

}
