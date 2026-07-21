#include <BlockNames.hpp>
#include <BlockIds.hpp>
#include <Console.hpp>
#include <mutex>
#include <set>

namespace {
    std::mutex loggedNamesMutex;
    std::set<string> loggedUnknownNames;
}

namespace BlockNames {

Int32 blockNameToStateId(const string& name, bool snowy) {
    if (name.empty() || name == "minecraft:air") return AIR_BLOCK_STATE_ID;
    if (name == "minecraft:stone") return STONE_BLOCK_STATE_ID;
    if (name == "minecraft:dirt") return DIRT_BLOCK_STATE_ID;
    if (name == "minecraft:grass_block" && !snowy) return GRASS_BLOCK_STATE_ID;

    std::lock_guard<std::mutex> lock(loggedNamesMutex);
    if (loggedUnknownNames.insert(name).second) {
        Console::getConsole().Entry("BlockNames::blockNameToStateId(): Unknown/unsupported block '" + name + "', substituting stone.");
    }
    return STONE_BLOCK_STATE_ID;
}

string blockStateIdToName(Int32 blockStateId) {
    switch (blockStateId) {
        case AIR_BLOCK_STATE_ID: return "minecraft:air";
        case STONE_BLOCK_STATE_ID: return "minecraft:stone";
        case DIRT_BLOCK_STATE_ID: return "minecraft:dirt";
        case GRASS_BLOCK_STATE_ID: return "minecraft:grass_block";
        default: return "minecraft:stone"; // shouldn't occur -- every ID this project produces is one of the above
    }
}

}
