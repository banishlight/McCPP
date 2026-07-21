#pragma once
#include <Standards.hpp>
#include <optional>

// Minimal (not byte-perfect-vanilla) level.dat read/write: just the fields
// this project actually models (seed, spawn, level name, game type,
// difficulty, hardcore, day time). Game rules, world border, datapacks, etc.
// are out of scope -- simply never read or written.
struct LevelData {
    Int64 seed = 0;
    string levelName = "world";
    double spawnX = 0.5;
    double spawnY = 64.0;
    double spawnZ = 0.5;
    float spawnYaw = 0.0f;
    int gameType = 0; // matches Player::getGamemode()'s 0-3 encoding
    int difficulty = 1; // matches Properties::Difficulty's enum order (Peaceful=0..Hard=3)
    bool hardcore = false;
    Int64 dayTime = 0;
    Int32 dataVersion = 0;
};

namespace LevelDat {
    // Reads <worldDir>/level.dat. std::nullopt if the file doesn't exist (a
    // brand-new world -- not an error) or if its DataVersion is below
    // VanillaVersion::MIN_SUPPORTED_DATA_VERSION (a clear "unsupported world
    // version" is logged in that case).
    std::optional<LevelData> tryLoad(const string& worldDir);

    // Writes <worldDir>/level.dat via write-to-temp-then-rename, so a crash
    // mid-save can't leave a half-written (unreadable) file behind.
    void save(const string& worldDir, const LevelData& data);
}
