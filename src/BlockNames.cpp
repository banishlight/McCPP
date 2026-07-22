#include <BlockNames.hpp>
#include <BlockIds.hpp>
#include <BlockTable.hpp>
#include <Console.hpp>
#include <mutex>
#include <set>
#include <algorithm>

namespace {
    std::mutex loggedNamesMutex;
    std::set<string> loggedUnknownNames;
}

namespace BlockNames {

Int32 blockNameToStateId(const string& name, bool snowy) {
    if (name.empty() || name == "minecraft:air") return AIR_BLOCK_STATE_ID;
    // grass_block's snowy state isn't modeled (this project only has the
    // default, non-snowy variant) -- handled entirely by its own branch
    // rather than the table scan below, since grass_block *is* a table entry
    // (representing only the non-snowy state) and would otherwise incorrectly
    // match regardless of snowy.
    if (name == "minecraft:grass_block") {
        if (!snowy) return GRASS_BLOCK_STATE_ID;
        // snowy grass_block falls through to the unknown/log path below.
    } else {
        const std::vector<BlockTableEntry>& table = getBlockTable();
        auto it = std::find_if(table.begin(), table.end(), [&name](const BlockTableEntry& entry) {
            return name == entry.name;
        });
        if (it != table.end()) return it->blockStateId;
    }

    std::lock_guard<std::mutex> lock(loggedNamesMutex);
    if (loggedUnknownNames.insert(name).second) {
        Console::getConsole().Entry("BlockNames::blockNameToStateId(): Unknown/unsupported block '" + name + "', substituting stone.");
    }
    return STONE_BLOCK_STATE_ID;
}

string blockStateIdToName(Int32 blockStateId) {
    if (blockStateId == AIR_BLOCK_STATE_ID) return "minecraft:air";

    const std::vector<BlockTableEntry>& table = getBlockTable();
    auto it = std::find_if(table.begin(), table.end(), [blockStateId](const BlockTableEntry& entry) {
        return blockStateId == entry.blockStateId;
    });
    if (it != table.end()) return it->name;

    return "minecraft:stone"; // shouldn't occur -- every ID this project produces is a known one
}

}
