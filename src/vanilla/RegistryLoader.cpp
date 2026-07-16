#include <vanilla/RegistryLoader.hpp>
#include <Zip.hpp>
#include <Standards.hpp>
#include <Console.hpp>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

namespace {
    const std::vector<string> REQUIRED_REGISTRIES = {
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
    const string SERVER_JAR_PATH = "server.jar";

    std::vector<Byte> readWholeFile(const string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return std::vector<Byte>();
        }
        return std::vector<Byte>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    string trimTrailingWhitespace(string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) {
            s.pop_back();
        }
        return s;
    }

    bool extractFromServerJar() {
        Console::getConsole().Entry("RegistryLoader: " + REGISTRY_DATA_DIR + " not found, attempting to extract from " + SERVER_JAR_PATH + "...");

        std::vector<Byte> outerBytes = readWholeFile(SERVER_JAR_PATH);
        if (outerBytes.empty()) {
            Console::getConsole().Error("RegistryLoader: Could not read " + SERVER_JAR_PATH + ". Place a vanilla 1.21 server.jar next to the server binary and restart.");
            return false;
        }

        ZipArchive outer;
        if (!outer.load(std::move(outerBytes))) {
            Console::getConsole().Error("RegistryLoader: Failed to parse " + SERVER_JAR_PATH + " as a ZIP archive.");
            return false;
        }

        std::vector<Byte> versionsListBytes = outer.extract("META-INF/versions.list");
        if (versionsListBytes.empty()) {
            Console::getConsole().Error("RegistryLoader: server.jar does not look like a modern (1.18+) bundler jar (missing META-INF/versions.list).");
            return false;
        }
        string versionsList(versionsListBytes.begin(), versionsListBytes.end());
        size_t lastTab = versionsList.find_last_of('\t');
        if (lastTab == string::npos) {
            Console::getConsole().Error("RegistryLoader: Could not parse META-INF/versions.list.");
            return false;
        }
        string nestedRelPath = trimTrailingWhitespace(versionsList.substr(lastTab + 1));
        string nestedFullPath = "META-INF/versions/" + nestedRelPath;

        std::vector<Byte> nestedBytes = outer.extract(nestedFullPath);
        if (nestedBytes.empty()) {
            Console::getConsole().Error("RegistryLoader: Failed to extract nested server jar at " + nestedFullPath);
            return false;
        }

        ZipArchive nested;
        if (!nested.load(std::move(nestedBytes))) {
            Console::getConsole().Error("RegistryLoader: Failed to parse nested server jar as a ZIP archive.");
            return false;
        }

        bool allOk = true;
        for (const string& registry : REQUIRED_REGISTRIES) {
            string prefix = "data/minecraft/" + registry + "/";
            std::vector<string> entries = nested.listEntries(prefix);
            if (entries.empty()) {
                Console::getConsole().Error("RegistryLoader: No entries found for registry " + registry + " (expected under " + prefix + ")");
                allOk = false;
                continue;
            }
            std::filesystem::path outDir = std::filesystem::path(REGISTRY_DATA_DIR) / registry;
            std::filesystem::create_directories(outDir);
            for (const string& entryName : entries) {
                if (entryName.size() < 5 || entryName.compare(entryName.size() - 5, 5, ".json") != 0) {
                    continue;
                }
                std::vector<Byte> jsonBytes = nested.extract(entryName);
                string baseName = entryName.substr(prefix.size());
                std::filesystem::path outPath = outDir / baseName;
                std::filesystem::create_directories(outPath.parent_path());
                std::ofstream outFile(outPath, std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(jsonBytes.data()), static_cast<std::streamsize>(jsonBytes.size()));
            }
            Console::getConsole().Entry("RegistryLoader: Extracted " + std::to_string(entries.size()) + " entries for " + registry);
        }

        return allOk;
    }
}

bool ensureRegistryDataExtracted() {
    if (std::filesystem::exists(REGISTRY_DATA_DIR) && std::filesystem::is_directory(REGISTRY_DATA_DIR)) {
        return true;
    }
    return extractFromServerJar();
}