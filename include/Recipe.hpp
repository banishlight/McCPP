#pragma once
#include <Standards.hpp>
#include <Player.hpp>
#include <array>

// Matches a crafting grid -- the player's own 2x2 personal grid (Player
// slots 1-4), or a crafting table's 3x3 grid (Player slots
// CRAFTING_TABLE_GRID_START..+9) -- against a small, real recipe set.
namespace Recipe {
    // Row-major view of the 2x2 grid: grid[0]=slot 1 (top-left),
    // grid[1]=slot 2 (top-right), grid[2]=slot 3 (bottom-left),
    // grid[3]=slot 4 (bottom-right).
    using Grid2x2 = std::array<InventorySlot, 4>;
    // Row-major view of a crafting table's 3x3 grid: grid[0]=top-left,
    // grid[8]=bottom-right.
    using Grid3x3 = std::array<InventorySlot, 9>;

    // Checks grid against every registered recipe. Returns false
    // (outResultItemId/outResultCount untouched) if nothing matches. Pure
    // function, no Player dependency beyond the InventorySlot value type --
    // callable from an offline scratch program with a hand-built grid.
    bool Match(const Grid2x2& grid, Int32& outResultItemId, Int32& outResultCount);
    bool Match3x3(const Grid3x3& grid, Int32& outResultItemId, Int32& outResultCount);

    // Reads player's slots 1-4, calls Match, and writes slot 0 (the derived
    // result) to the matched result or to empty if nothing matches. Never
    // touches slots 1-4 themselves.
    void RecomputeCraftingResult(Player& player);
    // Same, for a crafting table's 3x3 grid/result slots. Harmless no-op
    // (clears an already-empty result) when no crafting table window is
    // open, since the grid slots are simply all empty then.
    void RecomputeCraftingTableResult(Player& player);
}
