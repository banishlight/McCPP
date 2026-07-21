#include <systems/ItemPhysicsSystem.hpp>
#include <entities/ItemEntityManager.hpp>
#include <network/packets/Play.hpp>
#include <World.hpp>
#include <Chunk.hpp>
#include <BlockIds.hpp>
#include <cmath>
#include <algorithm>

void ItemPhysicsSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    ItemEntityManager& manager = ItemEntityManager::getInstance();

    for (const ItemEntity& entity : manager.snapshot()) {
        int chunkX = floorDiv16(static_cast<int>(std::floor(entity.x)));
        int chunkZ = floorDiv16(static_cast<int>(std::floor(entity.z)));
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) continue; // chunk not loaded -- leave it be, matches World::setBlock's own stance

        double candidateY = entity.y + entity.vy;
        int checkBlockY = static_cast<int>(std::floor(candidateY - 0.001));
        checkBlockY = std::max(checkBlockY, Chunk::WORLD_MIN_Y);
        checkBlockY = std::min(checkBlockY, Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1);

        int localX = static_cast<int>(std::floor(entity.x)) - chunkX * 16;
        int localZ = static_cast<int>(std::floor(entity.z)) - chunkZ * 16;
        Int32 blockBelow = chunk->getBlock(localX, checkBlockY, localZ);

        double newX = entity.x, newY = entity.y, newZ = entity.z;
        double newVx = entity.vx, newVy = entity.vy, newVz = entity.vz;
        bool onGround;

        if (blockBelow != AIR_BLOCK_STATE_ID || checkBlockY <= Chunk::WORLD_MIN_Y) {
            // Solid support (or the world floor): rest exactly on top of it.
            newY = checkBlockY + 1;
            newVx = newVy = newVz = 0.0;
            onGround = true;
        } else {
            newY = candidateY;
            newX = entity.x + entity.vx;
            newZ = entity.z + entity.vz;
            newVy = entity.vy - GRAVITY;
            newVx = entity.vx * DRAG;
            newVz = entity.vz * DRAG;
            onGround = false;
        }

        if (newX == entity.x && newY == entity.y && newZ == entity.z &&
            newVx == entity.vx && newVy == entity.vy && newVz == entity.vz) {
            continue; // already settled, nothing changed -- no update, no packet
        }

        int newChunkX = floorDiv16(static_cast<int>(std::floor(newX)));
        int newChunkZ = floorDiv16(static_cast<int>(std::floor(newZ)));
        // Internal bookkeeping (pickup-radius/despawn accuracy) stays authoritative
        // every tick, independent of whether clients get a correction this tick.
        manager.updatePosition(entity.entityId, newX, newY, newZ, newVx, newVy, newVz, newChunkX, newChunkZ);

        // Broadcasting a position correction every tick fights the client's own
        // local gravity simulation of this entity: only re-sync when velocity
        // changes by more than 0.1 units squared, trusting the client to
        // extrapolate the rest -- sending every tick caused visible stutter,
        // especially for the larger per-tick steps of a horizontal toss.
        double dvx = newVx - entity.vx, dvy = newVy - entity.vy, dvz = newVz - entity.vz;
        bool velocityDirty = (dvx * dvx + dvy * dvy + dvz * dvz) > 0.1;
        if (velocityDirty) {
            int entityId = entity.entityId;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newX, newY, newZ, onGround](int threshold) {
                return std::make_shared<Teleport_Entity_p>(threshold, entityId, newX, newY, newZ, 0.0f, 0.0f, onGround);
            });
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newVx, newVy, newVz](int threshold) {
                return std::make_shared<Set_Entity_Velocity_p>(threshold, entityId, newVx, newVy, newVz);
            });
        }
    }
}

string ItemPhysicsSystem::getName() const {
    return "ItemPhysics";
}
