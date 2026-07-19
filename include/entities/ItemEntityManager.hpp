#pragma once
#include <entities/ItemEntity.hpp>
#include <vector>
#include <mutex>

// Requires initialization: none -- singleton, lazily constructed like the
// project's other managers (ConnectionManager, World).
class ItemEntityManager {
    public:
        static ItemEntityManager& getInstance();
        // Allocates an entity ID (via EntityIdAllocator) and tracks a new dropped
        // item. Returns the full record so the caller has what it needs to build
        // the one-time Spawn_Entity_p/Set_Entity_Metadata_p broadcast. vx/vy/vz
        // default to 0 (break-drops fall straight down); Q-drop passes a toss vector.
        ItemEntity spawn(Int32 itemId, Int32 count, double x, double y, double z, int chunkX, int chunkZ,
                          double vx = 0.0, double vy = 0.0, double vz = 0.0);
        // Atomic remove-if-present: true if this call removed the entity (caller
        // now owns it), false if it was already gone (despawned or claimed by
        // someone else first). Prevents two players picking up the same item.
        bool tryClaim(int entityId);
        // Locked find-and-mutate for ItemPhysicsSystem. Silently no-ops if the
        // entity was already removed (picked up/despawned) since the caller's
        // last snapshot() -- same race-tolerant stance as tryClaim.
        void updatePosition(int entityId, double x, double y, double z, double vx, double vy, double vz, int chunkX, int chunkZ);
        // Snapshot for callers to iterate outside the lock -- mirrors
        // ConnectionManager::getActiveConnections().
        std::vector<ItemEntity> snapshot();
    private:
        ItemEntityManager() = default;
        std::vector<ItemEntity> _entities;
        std::mutex _mutex;
};
