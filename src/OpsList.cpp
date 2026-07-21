#include <OpsList.hpp>
#include <Console.hpp>
#include <lib/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace {
    const string OPS_FILE = "ops.json";
}

OpsList& OpsList::getInstance() {
    static OpsList instance;
    return instance;
}

void OpsList::initialize() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::ifstream file(OPS_FILE);
    if (!file.is_open()) {
        return; // No ops.json yet -- nobody is opped, matches a fresh vanilla server.
    }
    json parsed;
    try {
        file >> parsed;
    } catch (const std::exception& e) {
        Console::getConsole().Error("OpsList::initialize(): Failed to parse " + OPS_FILE + ": " + e.what());
        return;
    }
    if (!parsed.is_array()) return;
    for (const auto& entry : parsed) {
        if (!entry.contains("uuid") || !entry.contains("name") || !entry.contains("level")) continue;
        string uuid = entry["uuid"].get<string>();
        _entries[uuid] = Entry{entry["name"].get<string>(), entry["level"].get<int>()};
    }
}

int OpsList::getOpLevel(const string& uuidHex) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _entries.find(uuidHex);
    if (it == _entries.end()) return 0;
    return it->second.level;
}

void OpsList::setOpLevel(const string& uuidHex, const string& name, int level) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (level <= 0) {
            _entries.erase(uuidHex);
        } else {
            _entries[uuidHex] = Entry{name, level};
        }
    }
    save();
}

void OpsList::save() const {
    std::lock_guard<std::mutex> lock(_mutex);
    json array = json::array();
    for (const auto& [uuid, entry] : _entries) {
        array.push_back({{"uuid", uuid}, {"name", entry.name}, {"level", entry.level}});
    }
    std::ofstream file(OPS_FILE);
    file << array.dump(2);
}
