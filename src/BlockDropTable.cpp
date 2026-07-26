#include <BlockDropTable.hpp>
#include <BlockIds.hpp>
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

    struct DropTableEntry {
        Int32 blockStateId;
        Int32 itemId;
        double chance; // 0.0-1.0, base chance with no tool applied
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
}

namespace BlockDropTable {

bool CheckDrop(Int32 blockStateId, const ToolInfo& tool, Int32& outItemId) {
    if (tool.silkTouch) {
        // Silk Touch bypasses the table entirely -- every block drops
        // itself, including ones with no entry above at all.
        outItemId = blockStateIdToItemId(blockStateId);
        return true;
    }
    for (const DropTableEntry& entry : getDropTable()) {
        if (entry.blockStateId == blockStateId) {
            // tool.fortuneLevel is intentionally unused for now -- see the
            // CHECKLIST comment above getDropTable().
            outItemId = (rollChance() < entry.chance) ? entry.itemId : -1;
            return true;
        }
    }
    return false;
}

}
