#pragma once
#include <Standards.hpp>
#include <Player.hpp>
#include <array>

// Matches the player's own 2x2 personal-inventory crafting grid (Player
// slots 1-4) against a small, real recipe set -- manual placement only, no
// crafting-table block, no separate window, no recipe-book packets.
namespace Recipe {
    // Row-major view of the 2x2 grid: grid[0]=slot 1 (top-left),
    // grid[1]=slot 2 (top-right), grid[2]=slot 3 (bottom-left),
    // grid[3]=slot 4 (bottom-right).
    using Grid2x2 = std::array<InventorySlot, 4>;

    // Checks grid against every registered recipe. Returns false
    // (outResultItemId/outResultCount untouched) if nothing matches. Pure
    // function, no Player dependency beyond the InventorySlot value type --
    // callable from an offline scratch program with a hand-built Grid2x2.
    bool Match(const Grid2x2& grid, Int32& outResultItemId, Int32& outResultCount);

    // Reads player's slots 1-4, calls Match, and writes slot 0 (the derived
    // result) to the matched result or to empty if nothing matches. Never
    // touches slots 1-4 themselves.
    void RecomputeCraftingResult(Player& player);
}
