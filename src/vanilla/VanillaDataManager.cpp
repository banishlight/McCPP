#include <vanilla/VanillaDataManager.hpp>
#include <vanilla/RegistryLoader.hpp>
#include <Console.hpp>
#include <lib/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace {
    const std::vector<string> REGISTRY_NAMES = {
        "dimension_type",
        "worldgen/biome",
        "damage_type",
        "trim_pattern",
        "trim_material",
        "wolf_variant",
        "painting_variant",
        "banner_pattern",
        "chat_type",
        "jukebox_song",
    };
    const string REGISTRY_DATA_DIR = "registry_data";
}

VanillaDataManager& VanillaDataManager::getInstance() {
    static VanillaDataManager instance;
    return instance;
}

void VanillaDataManager::initialize() {
    if (!ensureRegistryDataExtracted()) {
        Console::getConsole().Error("VanillaDataManager::initialize(): Registry data unavailable; Configuration state will fail for connecting clients until server.jar is supplied.");
    }

    _registryNames = REGISTRY_NAMES;
    for (const string& registryName : REGISTRY_NAMES) {
        std::filesystem::path dir = std::filesystem::path(REGISTRY_DATA_DIR) / registryName;
        std::vector<RegistryEntry> entries;
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            Console::getConsole().Error("VanillaDataManager::initialize(): Missing registry directory " + dir.string());
            _cache[registryName] = entries;
            continue;
        }
        for (const auto& fileEntry : std::filesystem::directory_iterator(dir)) {
            if (!fileEntry.is_regular_file() || fileEntry.path().extension() != ".json") {
                continue;
            }
            std::ifstream file(fileEntry.path());
            json parsed;
            try {
                file >> parsed;
            } catch (const std::exception& e) {
                Console::getConsole().Error("VanillaDataManager::initialize(): Failed to parse " + fileEntry.path().string() + ": " + e.what());
                continue;
            }
            if (registryName == "worldgen/biome") {
                // These are pure server-side world-generation instructions (which
                // carver digs caves, which features/spawners/spawn rates apply) --
                // the client never generates its own terrain, so it has no use for
                // them. Sending the full raw datapack JSON (with these fields'
                // often-huge nested feature lists) bloats the registry sync and
                // risks the client's biome codec choking on structure it doesn't
                // expect once it actually needs biome data for real terrain.
                for (const char* genOnlyField : {"carvers", "features", "spawners", "spawn_costs", "creature_spawn_probability"}) {
                    parsed.erase(genOnlyField);
                }
            }
            string id = "minecraft:" + fileEntry.path().stem().string();
            entries.push_back(RegistryEntry{id, true, NbtTag::fromJson(parsed)});
        }
        Console::getConsole().Entry("VanillaDataManager::initialize(): Loaded " + std::to_string(entries.size()) + " entries for " + registryName);
        _cache[registryName] = std::move(entries);
    }
}

const std::vector<string>& VanillaDataManager::getRegistryNames() const {
    return _registryNames;
}

const std::vector<RegistryEntry>& VanillaDataManager::getEntries(const string& registryName) const {
    static const std::vector<RegistryEntry> empty;
    auto it = _cache.find(registryName);
    if (it == _cache.end()) {
        return empty;
    }
    return it->second;
}