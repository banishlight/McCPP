#pragma once
#include <entities/FallingBlockEntity.hpp>
#include <vector>
#include <mutex>

// Requires initialization: none -- singleton, lazily constructed like
// ItemEntityManager/ConnectionManager/World.
class FallingBlockEntityManager {
    public:
        static FallingBlockEntityManager& getInstance();
        // Allocates an entity ID (via EntityIdAllocator) and tracks a new
        // falling block. Returns the full record so the caller has what it
        // needs to build the one-time Spawn_Entity_p broadcast.
        FallingBlockEntity spawn(Int32 blockStateId, double x, double y, double z, int chunkX, int chunkZ);
        // Locked find-and-mutate for FallingBlockSystem. Silently no-ops if
        // the entity was already removed since the caller's last snapshot().
        void updatePosition(int entityId, double y, double vy, int chunkX, int chunkZ);
        // Landed (converted back into a real block) -- unlike ItemEntityManager's
        // tryClaim, there's no "race to pick up" concept here, since only
        // FallingBlockSystem itself ever removes an entry.
        void remove(int entityId);
        // Snapshot for callers to iterate outside the lock -- mirrors
        // ItemEntityManager::snapshot().
        std::vector<FallingBlockEntity> snapshot();
    private:
        FallingBlockEntityManager() = default;
        std::vector<FallingBlockEntity> _entities;
        std::mutex _mutex;
};
