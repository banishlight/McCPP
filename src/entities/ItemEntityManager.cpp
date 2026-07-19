#include <entities/ItemEntityManager.hpp>
#include <EntityIdAllocator.hpp>
#include <algorithm>

ItemEntityManager& ItemEntityManager::getInstance() {
    static ItemEntityManager instance;
    return instance;
}

ItemEntity ItemEntityManager::spawn(Int32 itemId, Int32 count, double x, double y, double z, int chunkX, int chunkZ,
                                     double vx, double vy, double vz) {
    ItemEntity entity{EntityIdAllocator::next(), itemId, count, x, y, z, vx, vy, vz, chunkX, chunkZ, std::chrono::steady_clock::now()};
    std::lock_guard<std::mutex> lock(_mutex);
    _entities.push_back(entity);
    return entity;
}

bool ItemEntityManager::tryClaim(int entityId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_entities.begin(), _entities.end(), [entityId](const ItemEntity& e) {
        return e.entityId == entityId;
    });
    if (it == _entities.end()) return false;
    _entities.erase(it);
    return true;
}

void ItemEntityManager::updatePosition(int entityId, double x, double y, double z, double vx, double vy, double vz, int chunkX, int chunkZ) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_entities.begin(), _entities.end(), [entityId](const ItemEntity& e) {
        return e.entityId == entityId;
    });
    if (it == _entities.end()) return;
    it->x = x; it->y = y; it->z = z;
    it->vx = vx; it->vy = vy; it->vz = vz;
    it->chunkX = chunkX; it->chunkZ = chunkZ;
}

std::vector<ItemEntity> ItemEntityManager::snapshot() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _entities;
}
