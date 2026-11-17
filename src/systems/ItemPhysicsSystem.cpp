#include <systems/ItemPhysicsSystem.hpp>
#include <systems/ItemPhysics.hpp>
#include <entities/ItemEntityManager.hpp>
#include <network/packets/Play.hpp>
#include <entities/EntityTracker.hpp>
#include <World.hpp>
#include <Chunk.hpp>
#include <cmath>

namespace {
    // Vanilla's tracking settings for item entities.
    constexpr EntityTracker::Config ITEM_TRACKING{20, true};

    // Live-world adapter for ItemPhysics::step, reading from whatever chunks
    // are currently cached.
    class WorldBlockSource : public ItemPhysics::BlockSource {
        public:
            explicit WorldBlockSource(World& world) : _world(world) {}
            bool getBlock(int x, int y, int z, Int32& outBlockStateId) const override {
                int chunkX = floorDiv16(x), chunkZ = floorDiv16(z);
                std::shared_ptr<Chunk> chunk = _world.getCachedChunk(chunkX, chunkZ);
                if (!chunk) return false;
                outBlockStateId = chunk->getBlock(x - chunkX * 16, y, z - chunkZ * 16);
                return true;
            }
        private:
            World& _world;
    };
}

void ItemPhysicsSystem::onTick(Int64 tickCount) {
    WorldBlockSource blocks(World::getInstance());
    ItemEntityManager& manager = ItemEntityManager::getInstance();

    for (const ItemEntity& entity : manager.snapshot()) {
        ItemPhysics::StepResult result;
        ItemPhysics::State current{entity.x, entity.y, entity.z, entity.vx, entity.vy, entity.vz};
        if (!ItemPhysics::step(blocks, current, result)) continue; // chunk not loaded -- leave it be

        const double newX = result.next.x, newY = result.next.y, newZ = result.next.z;
        const double newVx = result.next.vx, newVy = result.next.vy, newVz = result.next.vz;
        const bool onGround = result.onGround;

        if (newX == entity.x && newY == entity.y && newZ == entity.z &&
            newVx == entity.vx && newVy == entity.vy && newVz == entity.vz) {
            continue; // already settled, nothing changed -- no update, no packet
        }

        int newChunkX = floorDiv16(static_cast<int>(std::floor(newX)));
        int newChunkZ = floorDiv16(static_cast<int>(std::floor(newZ)));
        // Internal bookkeeping (pickup-radius/despawn accuracy) stays authoritative
        // every tick, independent of whether clients get a correction this tick.
        manager.updatePosition(entity.entityId, newX, newY, newZ, newVx, newVy, newVz, newChunkX, newChunkZ);

        // Send what vanilla's tracker would send this tick. In a fluid that's
        // an update every tick: the client simulates the item too, and only
        // per-tick corrections keep its copy from running ahead and getting
        // yanked back.
        int entityId = entity.entityId;
        EntityTracker::State tracker = entity.tracker;
        EntityTracker::Update update = EntityTracker::tick(tracker, ITEM_TRACKING, newX, newY, newZ,
                                                           newVx, newVy, newVz, onGround, result.hasImpulse);
        if (update.sendVelocity) {
            const double vx = update.vx, vy = update.vy, vz = update.vz;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, vx, vy, vz](int threshold) {
                return std::make_shared<Set_Entity_Velocity_p>(threshold, entityId, vx, vy, vz);
            });
        }
        if (update.position == EntityTracker::PositionPacket::Delta) {
            const Int16 dx = update.deltaX, dy = update.deltaY, dz = update.deltaZ;
            const bool ground = update.onGround;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, dx, dy, dz, ground](int threshold) {
                return std::make_shared<Update_Entity_Position_p>(threshold, entityId, dx, dy, dz, ground);
            });
        } else if (update.position == EntityTracker::PositionPacket::Teleport) {
            const double x = update.x, y = update.y, z = update.z;
            const bool ground = update.onGround;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, x, y, z, ground](int threshold) {
                return std::make_shared<Teleport_Entity_p>(threshold, entityId, x, y, z, 0.0f, 0.0f, ground);
            });
        }
        manager.setTrackerState(entityId, tracker);
    }
}

string ItemPhysicsSystem::getName() const {
    return "ItemPhysics";
}
