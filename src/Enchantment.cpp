#include <Enchantment.hpp>
#include <ItemProperties.hpp>

namespace {
    // Real item id for elytra (sourced from server.jar --reports, matching
    // this project's existing hardcoded-real-id convention -- e.g.
    // WATER_BUCKET_ITEM_ID in Play.cpp). Needed because elytra is classified
    // as EquipmentSlot::Chest (it occupies the chest slot) but is NOT a real
    // "armor" item for enchantment purposes -- real vanilla's
    // enchantable/armor tag (used by Protection and friends) excludes it,
    // while enchantable/equippable (used by Binding Curse) includes it. This
    // is the one place that distinction actually matters.
    constexpr Int32 ELYTRA_ITEM_ID = 773;

    // Head/Chest/Legs/Feet, excluding elytra -- real vanilla's
    // enchantable/armor tag (Protection, Blast/Fire/Projectile Protection,
    // Thorns). Confirmed exactly 25 items (6 armor materials x 4 slots +
    // turtle_helmet), elytra deliberately excluded.
    bool isRealArmor(Int32 itemId) {
        if (itemId == ELYTRA_ITEM_ID) return false;
        switch (ItemProperties::getEquipmentSlot(itemId)) {
            case EquipmentSlot::Head:
            case EquipmentSlot::Chest:
            case EquipmentSlot::Legs:
            case EquipmentSlot::Feet:
                return true;
            default:
                return false;
        }
    }

    // Head/Chest/Legs/Feet, INCLUDING elytra -- real vanilla's
    // enchantable/equippable tag (Binding Curse). Also real vanilla includes
    // a handful of head-slot novelty items (mob heads, carved_pumpkin) --
    // not modeled, matching ItemProperties' own disclosed gap for those.
    bool isArmorOrElytra(Int32 itemId) {
        switch (ItemProperties::getEquipmentSlot(itemId)) {
            case EquipmentSlot::Head:
            case EquipmentSlot::Chest:
            case EquipmentSlot::Legs:
            case EquipmentSlot::Feet:
                return true;
            default:
                return false;
        }
    }

    // Any tool/weapon/ranged-weapon category, OR any equipment slot
    // (including offhand/shield and elytra) -- real vanilla's broad
    // enchantable/durability and enchantable/vanishing tags (Mending,
    // Unbreaking, Vanishing Curse). Real vanilla also includes a handful of
    // non-tool misc items (shears... no, shears has no durability; brush,
    // carrot_on_a_stick, flint_and_steel, warped_fungus_on_a_stick) and a
    // couple of novelty items (compass, carved_pumpkin, mob heads) -- not
    // modeled, disclosed gap (none of those have any feature relevance in
    // this project yet either).
    bool isAnyToolWeaponOrEquipment(Int32 itemId) {
        return ItemProperties::getItemCategory(itemId) != ItemCategory::None
            || ItemProperties::getEquipmentSlot(itemId) != EquipmentSlot::None;
    }

    bool categoryIn(Int32 itemId, std::initializer_list<ItemCategory> categories) {
        ItemCategory cat = ItemProperties::getItemCategory(itemId);
        for (ItemCategory c : categories) {
            if (cat == c) return true;
        }
        return false;
    }

    // Every Enchantment value, for availableFor() to iterate -- kept in sync
    // by hand with the enum (a compile-time array in enum-declaration order,
    // matching this project's existing INSTANT_BREAK_BLOCKS-style hand-
    // maintained-list convention rather than machinery to enumerate an enum).
    constexpr Enchantment ALL_ENCHANTMENTS[] = {
        Enchantment::AquaAffinity, Enchantment::BaneOfArthropods, Enchantment::BindingCurse,
        Enchantment::BlastProtection, Enchantment::Breach, Enchantment::Channeling,
        Enchantment::Density, Enchantment::DepthStrider, Enchantment::Efficiency,
        Enchantment::FeatherFalling, Enchantment::FireAspect, Enchantment::FireProtection,
        Enchantment::Flame, Enchantment::Fortune, Enchantment::FrostWalker, Enchantment::Impaling,
        Enchantment::Infinity, Enchantment::Knockback, Enchantment::Looting, Enchantment::Loyalty,
        Enchantment::LuckOfTheSea, Enchantment::Lure, Enchantment::Mending, Enchantment::Multishot,
        Enchantment::Piercing, Enchantment::Power, Enchantment::ProjectileProtection,
        Enchantment::Protection, Enchantment::Punch, Enchantment::QuickCharge,
        Enchantment::Respiration, Enchantment::Riptide, Enchantment::Sharpness,
        Enchantment::SilkTouch, Enchantment::Smite, Enchantment::SoulSpeed,
        Enchantment::SweepingEdge, Enchantment::SwiftSneak, Enchantment::Thorns,
        Enchantment::Unbreaking, Enchantment::VanishingCurse, Enchantment::WindBurst,
    };
}

namespace ItemEnchantments {

Int32 getMaxLevel(Enchantment enchantment) {
    switch (enchantment) {
        case Enchantment::AquaAffinity: return 1;
        case Enchantment::BaneOfArthropods: return 5;
        case Enchantment::BindingCurse: return 1;
        case Enchantment::BlastProtection: return 4;
        case Enchantment::Breach: return 4;
        case Enchantment::Channeling: return 1;
        case Enchantment::Density: return 5;
        case Enchantment::DepthStrider: return 3;
        case Enchantment::Efficiency: return 5;
        case Enchantment::FeatherFalling: return 4;
        case Enchantment::FireAspect: return 2;
        case Enchantment::FireProtection: return 4;
        case Enchantment::Flame: return 1;
        case Enchantment::Fortune: return 3;
        case Enchantment::FrostWalker: return 2;
        case Enchantment::Impaling: return 5;
        case Enchantment::Infinity: return 1;
        case Enchantment::Knockback: return 2;
        case Enchantment::Looting: return 3;
        case Enchantment::Loyalty: return 3;
        case Enchantment::LuckOfTheSea: return 3;
        case Enchantment::Lure: return 3;
        case Enchantment::Mending: return 1;
        case Enchantment::Multishot: return 1;
        case Enchantment::Piercing: return 4;
        case Enchantment::Power: return 5;
        case Enchantment::ProjectileProtection: return 4;
        case Enchantment::Protection: return 4;
        case Enchantment::Punch: return 2;
        case Enchantment::QuickCharge: return 3;
        case Enchantment::Respiration: return 3;
        case Enchantment::Riptide: return 3;
        case Enchantment::Sharpness: return 5;
        case Enchantment::SilkTouch: return 1;
        case Enchantment::Smite: return 5;
        case Enchantment::SoulSpeed: return 3;
        case Enchantment::SweepingEdge: return 3;
        case Enchantment::SwiftSneak: return 3;
        case Enchantment::Thorns: return 3;
        case Enchantment::Unbreaking: return 3;
        case Enchantment::VanishingCurse: return 1;
        case Enchantment::WindBurst: return 3;
    }
    return 1;
}

string getName(Enchantment enchantment) {
    switch (enchantment) {
        case Enchantment::AquaAffinity: return "minecraft:aqua_affinity";
        case Enchantment::BaneOfArthropods: return "minecraft:bane_of_arthropods";
        case Enchantment::BindingCurse: return "minecraft:binding_curse";
        case Enchantment::BlastProtection: return "minecraft:blast_protection";
        case Enchantment::Breach: return "minecraft:breach";
        case Enchantment::Channeling: return "minecraft:channeling";
        case Enchantment::Density: return "minecraft:density";
        case Enchantment::DepthStrider: return "minecraft:depth_strider";
        case Enchantment::Efficiency: return "minecraft:efficiency";
        case Enchantment::FeatherFalling: return "minecraft:feather_falling";
        case Enchantment::FireAspect: return "minecraft:fire_aspect";
        case Enchantment::FireProtection: return "minecraft:fire_protection";
        case Enchantment::Flame: return "minecraft:flame";
        case Enchantment::Fortune: return "minecraft:fortune";
        case Enchantment::FrostWalker: return "minecraft:frost_walker";
        case Enchantment::Impaling: return "minecraft:impaling";
        case Enchantment::Infinity: return "minecraft:infinity";
        case Enchantment::Knockback: return "minecraft:knockback";
        case Enchantment::Looting: return "minecraft:looting";
        case Enchantment::Loyalty: return "minecraft:loyalty";
        case Enchantment::LuckOfTheSea: return "minecraft:luck_of_the_sea";
        case Enchantment::Lure: return "minecraft:lure";
        case Enchantment::Mending: return "minecraft:mending";
        case Enchantment::Multishot: return "minecraft:multishot";
        case Enchantment::Piercing: return "minecraft:piercing";
        case Enchantment::Power: return "minecraft:power";
        case Enchantment::ProjectileProtection: return "minecraft:projectile_protection";
        case Enchantment::Protection: return "minecraft:protection";
        case Enchantment::Punch: return "minecraft:punch";
        case Enchantment::QuickCharge: return "minecraft:quick_charge";
        case Enchantment::Respiration: return "minecraft:respiration";
        case Enchantment::Riptide: return "minecraft:riptide";
        case Enchantment::Sharpness: return "minecraft:sharpness";
        case Enchantment::SilkTouch: return "minecraft:silk_touch";
        case Enchantment::Smite: return "minecraft:smite";
        case Enchantment::SoulSpeed: return "minecraft:soul_speed";
        case Enchantment::SweepingEdge: return "minecraft:sweeping_edge";
        case Enchantment::SwiftSneak: return "minecraft:swift_sneak";
        case Enchantment::Thorns: return "minecraft:thorns";
        case Enchantment::Unbreaking: return "minecraft:unbreaking";
        case Enchantment::VanishingCurse: return "minecraft:vanishing_curse";
        case Enchantment::WindBurst: return "minecraft:wind_burst";
    }
    return "";
}

// Each case mirrors that enchantment's real "supported_items" tag from its
// data/minecraft/enchantment/<name>.json, resolved down to this project's
// ItemCategory/EquipmentSlot classification (see Enchantment.hpp's own
// comment on the primary_items distinction this doesn't yet model).
bool canApply(Enchantment enchantment, Int32 itemId) {
    switch (enchantment) {
        case Enchantment::AquaAffinity:
        case Enchantment::Respiration:
            return ItemProperties::getEquipmentSlot(itemId) == EquipmentSlot::Head;
        case Enchantment::DepthStrider:
        case Enchantment::FeatherFalling:
        case Enchantment::FrostWalker:
        case Enchantment::SoulSpeed:
            return ItemProperties::getEquipmentSlot(itemId) == EquipmentSlot::Feet;
        case Enchantment::SwiftSneak:
            return ItemProperties::getEquipmentSlot(itemId) == EquipmentSlot::Legs;
        case Enchantment::BindingCurse:
            return isArmorOrElytra(itemId);
        case Enchantment::BlastProtection:
        case Enchantment::FireProtection:
        case Enchantment::ProjectileProtection:
        case Enchantment::Protection:
        case Enchantment::Thorns:
            return isRealArmor(itemId);
        case Enchantment::BaneOfArthropods:
        case Enchantment::Smite:
            return categoryIn(itemId, {ItemCategory::Sword, ItemCategory::Axe, ItemCategory::Mace});
        case Enchantment::FireAspect:
            return categoryIn(itemId, {ItemCategory::Sword, ItemCategory::Mace});
        case Enchantment::Sharpness:
            return categoryIn(itemId, {ItemCategory::Sword, ItemCategory::Axe});
        case Enchantment::Knockback:
        case Enchantment::Looting:
        case Enchantment::SweepingEdge:
            return categoryIn(itemId, {ItemCategory::Sword});
        case Enchantment::Efficiency:
            // Real vanilla also includes shears here -- disclosed gap, see
            // Enchantment.hpp/ItemTable.cpp's own comments.
            return categoryIn(itemId, {ItemCategory::Pickaxe, ItemCategory::Axe, ItemCategory::Shovel, ItemCategory::Hoe});
        case Enchantment::Fortune:
        case Enchantment::SilkTouch:
            return categoryIn(itemId, {ItemCategory::Pickaxe, ItemCategory::Axe, ItemCategory::Shovel, ItemCategory::Hoe});
        case Enchantment::Flame:
        case Enchantment::Infinity:
        case Enchantment::Power:
        case Enchantment::Punch:
            return categoryIn(itemId, {ItemCategory::Bow});
        case Enchantment::Multishot:
        case Enchantment::Piercing:
        case Enchantment::QuickCharge:
            return categoryIn(itemId, {ItemCategory::Crossbow});
        case Enchantment::Channeling:
        case Enchantment::Impaling:
        case Enchantment::Loyalty:
        case Enchantment::Riptide:
            return categoryIn(itemId, {ItemCategory::Trident});
        case Enchantment::Breach:
        case Enchantment::Density:
        case Enchantment::WindBurst:
            return categoryIn(itemId, {ItemCategory::Mace});
        case Enchantment::LuckOfTheSea:
        case Enchantment::Lure:
            return categoryIn(itemId, {ItemCategory::FishingRod});
        case Enchantment::Mending:
        case Enchantment::Unbreaking:
        case Enchantment::VanishingCurse:
            return isAnyToolWeaponOrEquipment(itemId);
    }
    return false;
}

std::vector<Enchantment> availableFor(Int32 itemId) {
    std::vector<Enchantment> result;
    for (Enchantment enchantment : ALL_ENCHANTMENTS) {
        if (canApply(enchantment, itemId)) {
            result.push_back(enchantment);
        }
    }
    return result;
}

}
