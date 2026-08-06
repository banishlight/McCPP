#include <Recipe.hpp>
#include <ItemNames.hpp>
#include <vector>

namespace {
    // An OR'd set of acceptable item ids for one ingredient slot -- covers a
    // literal item, a resolved tag (multiple ids), and a real recipe's "list
    // of alternatives" key (e.g. torch's coal-or-charcoal) with the same
    // shape.
    struct IngredientSet {
        std::vector<Int32> ids;
        bool matches(Int32 itemId) const {
            for (Int32 id : ids) if (id == itemId) return true;
            return false;
        }
    };

    struct ShapedRecipe {
        int width, height; // <= 2 each -- this project's crafting grid is 2x2
        std::vector<IngredientSet> cells; // row-major, size width*height
        Int32 resultItemId, resultCount;
    };

    struct ShapelessRecipe {
        std::vector<IngredientSet> ingredients; // one per required item; order irrelevant
        Int32 resultItemId, resultCount;
    };

    // Real recipe data pulled from data/minecraft/recipe/*.json inside the
    // nested META-INF/versions/1.21/server-1.21.jar within this project's own
    // bin/server.jar (not the --reports output -- recipes aren't in there).
    // Resolved once via ItemNames::itemNameToId (function-local static, same
    // lazy-init idiom BlockDropTable::getDropTable() uses for its own table)
    // rather than duplicating magic item-id integers the way
    // BlockDropTable.cpp does -- that file predates ItemNames existing as a
    // comprehensive, verified name->id lookup.
    const IngredientSet& oakLogsTag() {
        static const IngredientSet s = { {
            ItemNames::itemNameToId("minecraft:oak_log"),
            ItemNames::itemNameToId("minecraft:oak_wood"),
            ItemNames::itemNameToId("minecraft:stripped_oak_log"),
            ItemNames::itemNameToId("minecraft:stripped_oak_wood"),
        } };
        return s;
    }
    const IngredientSet& planksTag() {
        static const IngredientSet s = { {
            ItemNames::itemNameToId("minecraft:oak_planks"),
            ItemNames::itemNameToId("minecraft:spruce_planks"),
            ItemNames::itemNameToId("minecraft:birch_planks"),
            ItemNames::itemNameToId("minecraft:jungle_planks"),
            ItemNames::itemNameToId("minecraft:acacia_planks"),
            ItemNames::itemNameToId("minecraft:dark_oak_planks"),
            ItemNames::itemNameToId("minecraft:crimson_planks"),
            ItemNames::itemNameToId("minecraft:warped_planks"),
            ItemNames::itemNameToId("minecraft:mangrove_planks"),
            ItemNames::itemNameToId("minecraft:bamboo_planks"),
            ItemNames::itemNameToId("minecraft:cherry_planks"),
        } };
        return s;
    }
    const IngredientSet& coalOrCharcoal() {
        static const IngredientSet s = { {
            ItemNames::itemNameToId("minecraft:coal"),
            ItemNames::itemNameToId("minecraft:charcoal"),
        } };
        return s;
    }
    const IngredientSet& stickOnly() {
        static const IngredientSet s = { { ItemNames::itemNameToId("minecraft:stick") } };
        return s;
    }

    // *** ADD NEW SHAPED RECIPES HERE, ONE PER LINE ***
    const std::vector<ShapedRecipe>& getShapedRecipes() {
        static const std::vector<ShapedRecipe> table = {
            // stick: 1 wide x 2 tall, planks stacked vertically
            { 1, 2, { planksTag(), planksTag() },
              ItemNames::itemNameToId("minecraft:stick"), 4 },
            // crafting_table: 2x2 planks
            { 2, 2, { planksTag(), planksTag(), planksTag(), planksTag() },
              ItemNames::itemNameToId("minecraft:crafting_table"), 1 },
            // torch: 1 wide x 2 tall, coal/charcoal over a stick
            { 1, 2, { coalOrCharcoal(), stickOnly() },
              ItemNames::itemNameToId("minecraft:torch"), 4 },
        };
        return table;
    }

    // *** ADD NEW SHAPELESS RECIPES HERE, ONE PER LINE ***
    const std::vector<ShapelessRecipe>& getShapelessRecipes() {
        static const std::vector<ShapelessRecipe> table = {
            // oak_planks: only the oak log family is craftable here -- this
            // project's world generator only produces oak trees
            // (NoiseChunkGenerator's tree feature), so the other 10 species'
            // log->plank conversions are unreachable and deliberately
            // omitted. Trivially extensible later: one more ingredient-set
            // entry per species, not an architecture limitation.
            { { oakLogsTag() }, ItemNames::itemNameToId("minecraft:oak_planks"), 4 },
        };
        return table;
    }

    bool tryMatchShaped(const ShapedRecipe& r, const Recipe::Grid2x2& grid, Int32& outItemId, Int32& outCount) {
        int maxAnchorX = 2 - r.width;
        int maxAnchorY = 2 - r.height;
        for (int ax = 0; ax <= maxAnchorX; ax++) {
            for (int ay = 0; ay <= maxAnchorY; ay++) {
                bool ok = true;
                for (int gy = 0; gy < 2 && ok; gy++) {
                    for (int gx = 0; gx < 2 && ok; gx++) {
                        Int32 cellItem = grid[gy * 2 + gx].itemId;
                        int rx = gx - ax, ry = gy - ay;
                        if (rx >= 0 && rx < r.width && ry >= 0 && ry < r.height) {
                            if (!r.cells[ry * r.width + rx].matches(cellItem)) ok = false;
                        } else if (cellItem != -1) {
                            ok = false; // outside the pattern's footprint must be empty
                        }
                    }
                }
                if (ok) {
                    outItemId = r.resultItemId;
                    outCount = r.resultCount;
                    return true;
                }
            }
        }
        return false;
    }

    // Small backtracking assignment of occupied grid cells to required
    // ingredients -- a greedy first-fit isn't safe in general bipartite
    // matching, but a full backtrack over at most 4 cells is free.
    bool assignShapeless(const ShapelessRecipe& r, const Recipe::Grid2x2& grid, std::array<bool, 4>& used, size_t ingredientIdx) {
        if (ingredientIdx == r.ingredients.size()) return true;
        for (int cell = 0; cell < 4; cell++) {
            if (used[cell] || grid[cell].itemId == -1) continue;
            if (!r.ingredients[ingredientIdx].matches(grid[cell].itemId)) continue;
            used[cell] = true;
            if (assignShapeless(r, grid, used, ingredientIdx + 1)) return true;
            used[cell] = false;
        }
        return false;
    }

    bool tryMatchShapeless(const ShapelessRecipe& r, const Recipe::Grid2x2& grid, Int32& outItemId, Int32& outCount) {
        int occupiedCount = 0;
        for (const InventorySlot& slot : grid) if (slot.itemId != -1) occupiedCount++;
        if (occupiedCount != static_cast<int>(r.ingredients.size())) return false;

        std::array<bool, 4> used = { false, false, false, false };
        if (!assignShapeless(r, grid, used, 0)) return false;
        outItemId = r.resultItemId;
        outCount = r.resultCount;
        return true;
    }
}

bool Recipe::Match(const Grid2x2& grid, Int32& outResultItemId, Int32& outResultCount) {
    for (const ShapedRecipe& r : getShapedRecipes()) {
        if (tryMatchShaped(r, grid, outResultItemId, outResultCount)) return true;
    }
    for (const ShapelessRecipe& r : getShapelessRecipes()) {
        if (tryMatchShapeless(r, grid, outResultItemId, outResultCount)) return true;
    }
    return false;
}

void Recipe::RecomputeCraftingResult(Player& player) {
    Grid2x2 grid = { player.getSlot(1), player.getSlot(2), player.getSlot(3), player.getSlot(4) };
    Int32 itemId, count;
    if (Match(grid, itemId, count)) {
        player.setSlot(0, itemId, count);
    } else {
        player.setSlot(0, -1, 0);
    }
}
