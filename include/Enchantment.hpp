#pragma once
#include <Standards.hpp>
#include <vector>

// Every real vanilla enchantment (42 total, confirmed by listing
// data/minecraft/enchantment/*.json in a fresh server.jar --reports run's
// bundled nested server jar -- not guessed). No enchanting-table/anvil/
// command system exists yet to actually grant one of these -- this is data-
// only groundwork for that future feature (per the user's own request), the
// same "scaffolding with no live consumer yet" treatment BlockDropTable's
// ToolInfo got for Silk Touch/Fortune earlier.
enum class Enchantment {
    AquaAffinity, BaneOfArthropods, BindingCurse, BlastProtection, Breach, Channeling,
    Density, DepthStrider, Efficiency, FeatherFalling, FireAspect, FireProtection, Flame,
    Fortune, FrostWalker, Impaling, Infinity, Knockback, Looting, Loyalty, LuckOfTheSea, Lure,
    Mending, Multishot, Piercing, Power, ProjectileProtection, Protection, Punch, QuickCharge,
    Respiration, Riptide, Sharpness, SilkTouch, Smite, SoulSpeed, SweepingEdge, SwiftSneak,
    Thorns, Unbreaking, VanishingCurse, WindBurst,
};

namespace ItemEnchantments {
    // Real per-enchantment facts, sourced from each enchantment's own
    // data/minecraft/enchantment/<name>.json (max_level, and the name itself
    // is just the enum's real vanilla identifier) -- see Enchantment.cpp's
    // header comment.
    Int32 getMaxLevel(Enchantment enchantment);
    string getName(Enchantment enchantment); // "minecraft:sharpness" etc.

    // Whether `enchantment` can ever be applied to `itemId` at all -- matches
    // real vanilla's "supported_items" tag membership (the full applicability
    // used by anvils/commands/loot), NOT the narrower "primary_items" an
    // enchanting table specifically offers by default (a real, separate,
    // smaller set for a few enchantments -- e.g. Sharpness's supported_items
    // is swords+axes but its primary_items is swords only). That distinction
    // is deliberately not modeled yet -- refine when an actual enchanting
    // table is built and needs to know what to *offer*, not just what's
    // *legal*.
    bool canApply(Enchantment enchantment, Int32 itemId);

    // Every enchantment canApply() accepts for itemId, in the enum's
    // declared order -- what an enchanting-table implementation would query
    // to build its offered list.
    std::vector<Enchantment> availableFor(Int32 itemId);
}
