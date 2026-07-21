#pragma once
#include <Standards.hpp>
#include <Player.hpp>
#include <optional>
#include <array>
#include <vector>

// Minimal (not byte-perfect-vanilla) per-player save: just the Player fields
// that have a real analogue in vanilla's own playerdata/<uuid>.dat (position,
// rotation, gamemode, hotbar/selected slot). Health, XP, achievements, and a
// full 36-slot inventory don't exist as Player state today, so there's
// nothing to persist for them.
struct PlayerSaveData {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    float yaw = 0.0f;
    float pitch = 0.0f;
    int gamemode = 0;
    std::array<HotbarSlot, Player::HOTBAR_SIZE> hotbar{}; // defaults to all-empty ({-1, 0})
    int selectedSlot = 0;
};

// Free functions, matching LevelDat's shape -- no open-handle state to cache
// (unlike WorldPersistence's RegionFile handles), so no singleton is needed.
namespace PlayerDataPersistence {
    // Reads <worldDir>/playerdata/<dashed-uuid>.dat. std::nullopt if no file
    // exists yet for this UUID (a brand-new player -- not an error); caller
    // falls back to world-spawn defaults in that case.
    std::optional<PlayerSaveData> tryLoad(const string& worldDir, const std::vector<long>& uuid);

    // Writes <worldDir>/playerdata/<dashed-uuid>.dat via write-to-temp-then-rename
    // (same crash-safety convention as LevelDat::save). Safe to call from
    // multiple threads for different (or the same) player concurrently --
    // internally serialized by a single mutex, since a disconnect-triggered
    // save can plausibly race a periodic autosave for the same player.
    void save(const string& worldDir, const Player& player);
}
