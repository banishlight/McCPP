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
        manager.updatePosition(entity.entityId, newX, newY, newZ, newVx, newVy, newVz, newChunkX, newChunkZ);

        if (newX != entity.x || newY != entity.y || newZ != entity.z) {
            Int16 deltaX = static_cast<Int16>((newX - entity.x) * 4096.0);
            Int16 deltaY = static_cast<Int16>((newY - entity.y) * 4096.0);
            Int16 deltaZ = static_cast<Int16>((newZ - entity.z) * 4096.0);
            int entityId = entity.entityId;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, deltaX, deltaY, deltaZ, onGround](int threshold) {
                return std::make_shared<Update_Entity_Position_p>(threshold, entityId, deltaX, deltaY, deltaZ, onGround);
            });
        }
    }
}

string ItemPhysicsSystem::getName() const {
    return "ItemPhysics";
}
