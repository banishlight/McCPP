#pragma once
#include <network/Nbt.hpp>
#include <vector>
#include <string>
#include <map>

// Loads Configuration-state registry content (dimension_type, worldgen/biome,
// etc.) once at startup - extracting it from a vanilla server.jar first if
// registry_data/ doesn't already exist - and caches it in memory as NBT
// entries ready to hand to Registry_Data_p.
class VanillaDataManager {
    public:
        static VanillaDataManager& getInstance();
        // Extracts (if needed) and loads all registries into memory. Safe to call once at startup.
        void initialize();
        const std::vector<string>& getRegistryNames() const;
        const std::vector<RegistryEntry>& getEntries(const string& registryName) const;
    private:
        VanillaDataManager() = default;
        std::vector<string> _registryNames;
        std::map<string, std::vector<RegistryEntry>> _cache;
};