#include <systems/ItemDespawnSystem.hpp>
#include <entities/ItemEntityManager.hpp>
#include <network/packets/Play.hpp>
#include <chrono>

void ItemDespawnSystem::onTick(Int64 tickCount) {
    auto now = std::chrono::steady_clock::now();
    ItemEntityManager& manager = ItemEntityManager::getInstance();
    for (const ItemEntity& entity : manager.snapshot()) {
        double age = std::chrono::duration<double>(now - entity.spawnTime).count();
        if (age < DESPAWN_SECONDS) continue;
        if (!manager.tryClaim(entity.entityId)) continue; // already picked up between the snapshot and here
        BroadcastToChunkViewers(entity.chunkX, entity.chunkZ, [entityId = entity.entityId](int threshold) {
            return std::make_shared<Remove_Entities_p>(threshold, entityId);
        });
    }
}

string ItemDespawnSystem::getName() const {
    return "ItemDespawn";
}
