#pragma once
#include <Standards.hpp>

// The chunk/level.dat "DataVersion" this project writes, and the floor below
// which an imported save is rejected as unsupported (see docs, "Vanilla
// world-save (Anvil format) read/write" -- import targets 1.18+ only, since
// that's when vanilla's chunk format last matched this project's own
// WORLD_MIN_Y/WORLD_HEIGHT/section layout).
namespace VanillaVersion {
    // 1.21 (this project's target version, protocol 767) -- read directly
    // from this project's own server.jar's root version.json ("world_version"
    // field), not guessed.
    static constexpr Int32 CURRENT_DATA_VERSION = 3953;
    // Java Edition 1.18 release -- confirmed against minecraft.wiki's Data
    // version article, not recalled from memory alone.
    static constexpr Int32 MIN_SUPPORTED_DATA_VERSION = 2860;
}
