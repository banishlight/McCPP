#pragma once
#include <entities/ItemEntity.hpp>
#include <vector>
#include <mutex>

// Real vanilla pickup delays: 10 ticks (0.5s) for an ordinary block-break
// drop, 40 ticks (2.0s) for a player-initiated toss (Q-drop, dragging an
// item out of an inventory screen) -- the longer toss delay exists
// specifically so a player can't immediately re-pick-up what they just
// intentionally threw away, before it's had a chance to actually leave.
static constexpr double ITEM_PICKUP_DELAY_BREAK_SECONDS = 0.5;
static constexpr double ITEM_PICKUP_DELAY_TOSS_SECONDS = 2.0;

// Requires initialization: none -- singleton, lazily constructed like the
// project's other managers (ConnectionManager, World).
class ItemEntityManager {
    public:
        static ItemEntityManager& getInstance();
        // Allocates an entity ID (via EntityIdAllocator) and tracks a new dropped
        // item. Returns the full record so the caller has what it needs to build
        // the one-time Spawn_Entity_p/Set_Entity_Metadata_p broadcast. vx/vy/vz
        // default to 0 (break-drops fall straight down); Q-drop passes a toss vector.
        // pickupDelaySeconds defaults to the break-drop delay -- toss-style spawns
        // (Q-drop, inventory-screen drops) pass ITEM_PICKUP_DELAY_TOSS_SECONDS.
        ItemEntity spawn(Int32 itemId, Int32 count, double x, double y, double z, int chunkX, int chunkZ,
                          double vx = 0.0, double vy = 0.0, double vz = 0.0,
                          double pickupDelaySeconds = ITEM_PICKUP_DELAY_BREAK_SECONDS);
        // Atomic remove-if-present: true if this call removed the entity (caller
        // now owns it), false if it was already gone (despawned or claimed by
        // someone else first). Prevents two players picking up the same item.
        bool tryClaim(int entityId);
        // Locked find-and-mutate for ItemPhysicsSystem. Silently no-ops if the
        // entity was already removed (picked up/despawned) since the caller's
        // last snapshot() -- same race-tolerant stance as tryClaim.
        void updatePosition(int entityId, double x, double y, double z, double vx, double vy, double vz, int chunkX, int chunkZ);
        // Records that (x,y,z) was just sent to clients -- the baseline
        // ItemPhysicsSystem's fluid-push delta broadcasting needs, separate
        // from the internally-tracked current position, since those
        // corrections aren't sent every tick. Same silently-no-ops-if-gone
        // stance as updatePosition.
        void markPositionBroadcast(int entityId, double x, double y, double z);
        // Snapshot for callers to iterate outside the lock -- mirrors
        // ConnectionManager::getActiveConnections().
        std::vector<ItemEntity> snapshot();
    private:
        ItemEntityManager() = default;
        std::vector<ItemEntity> _entities;
        std::mutex _mutex;
};
