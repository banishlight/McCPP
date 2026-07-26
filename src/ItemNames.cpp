#include <ItemNames.hpp>
#include <ItemTable.hpp>
#include <Console.hpp>
#include <mutex>
#include <set>
#include <algorithm>

namespace {
    std::mutex loggedNamesMutex;
    std::set<string> loggedUnknownNames;
}

namespace ItemNames {

Int32 itemNameToId(const string& name) {
    if (name.empty()) return -1;

    const std::vector<ItemTableEntry>& table = getItemTable();
    auto it = std::find_if(table.begin(), table.end(), [&name](const ItemTableEntry& entry) {
        return name == entry.name;
    });
    if (it != table.end()) return it->itemId;

    std::lock_guard<std::mutex> lock(loggedNamesMutex);
    if (loggedUnknownNames.insert(name).second) {
        Console::getConsole().Entry("ItemNames::itemNameToId(): Unknown/unsupported item '" + name + "', treating slot as empty.");
    }
    return -1;
}

string itemIdToName(Int32 itemId) {
    if (itemId < 0) return "";

    const std::vector<ItemTableEntry>& table = getItemTable();
    auto it = std::find_if(table.begin(), table.end(), [itemId](const ItemTableEntry& entry) {
        return itemId == entry.itemId;
    });
    if (it != table.end()) return it->name;

    return ""; // shouldn't occur -- every id this project produces is a known one
}

}
