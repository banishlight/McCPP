#include <entities/FallingBlockEntityManager.hpp>
#include <EntityIdAllocator.hpp>
#include <algorithm>

FallingBlockEntityManager& FallingBlockEntityManager::getInstance() {
    static FallingBlockEntityManager instance;
    return instance;
}

FallingBlockEntity FallingBlockEntityManager::spawn(Int32 blockStateId, double x, double y, double z, int chunkX, int chunkZ) {
    FallingBlockEntity entity{EntityIdAllocator::next(), blockStateId, x, y, z, 0.0, chunkX, chunkZ};
    std::lock_guard<std::mutex> lock(_mutex);
    _entities.push_back(entity);
    return entity;
}

void FallingBlockEntityManager::updatePosition(int entityId, double y, double vy, int chunkX, int chunkZ) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_entities.begin(), _entities.end(), [entityId](const FallingBlockEntity& e) {
        return e.entityId == entityId;
    });
    if (it == _entities.end()) return;
    it->y = y;
    it->vy = vy;
    it->chunkX = chunkX;
    it->chunkZ = chunkZ;
}

void FallingBlockEntityManager::remove(int entityId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_entities.begin(), _entities.end(), [entityId](const FallingBlockEntity& e) {
        return e.entityId == entityId;
    });
    if (it == _entities.end()) return;
    _entities.erase(it);
}

std::vector<FallingBlockEntity> FallingBlockEntityManager::snapshot() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _entities;
}
