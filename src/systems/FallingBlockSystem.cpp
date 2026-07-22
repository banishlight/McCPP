#include <systems/FallingBlockSystem.hpp>
#include <entities/FallingBlockEntityManager.hpp>
#include <network/packets/Play.hpp>
#include <World.hpp>
#include <Chunk.hpp>
#include <BlockIds.hpp>
#include <cmath>
#include <algorithm>

void FallingBlockSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    FallingBlockEntityManager& manager = FallingBlockEntityManager::getInstance();

    for (const FallingBlockEntity& entity : manager.snapshot()) {
        int bx = static_cast<int>(std::floor(entity.x));
        int bz = static_cast<int>(std::floor(entity.z));
        int chunkX = floorDiv16(bx);
        int chunkZ = floorDiv16(bz);
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) continue; // chunk not loaded -- leave it be, matches ItemPhysicsSystem's own stance

        double candidateY = entity.y + entity.vy;
        int checkBlockY = static_cast<int>(std::floor(candidateY - 0.001));
        checkBlockY = std::max(checkBlockY, Chunk::WORLD_MIN_Y);
        checkBlockY = std::min(checkBlockY, Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1);

        int localX = bx - chunkX * 16;
        int localZ = bz - chunkZ * 16;
        Int32 blockBelow = chunk->getBlock(localX, checkBlockY, localZ);

        if (blockBelow != AIR_BLOCK_STATE_ID || checkBlockY <= Chunk::WORLD_MIN_Y) {
            // Landed: convert back into a real, static block and drop the entity.
            int landY = checkBlockY + 1;
            world.setBlock(bx, landY, bz, entity.blockStateId);
            Int32 blockStateId = entity.blockStateId;
            BroadcastToChunkViewers(chunkX, chunkZ, [bx, landY, bz, blockStateId](int threshold) {
                return std::make_shared<Block_Update_p>(threshold, bx, landY, bz, blockStateId);
            });
            int entityId = entity.entityId;
            manager.remove(entityId);
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId](int threshold) {
                return std::make_shared<Remove_Entities_p>(threshold, entityId);
            });
            continue;
        }

        double newY = candidateY;
        double newVy = entity.vy - GRAVITY;
        manager.updatePosition(entity.entityId, newY, newVy, chunkX, chunkZ);

        // Falling blocks are short-lived (a handful of ticks in the common
        // case), so a corrected position is sent every tick rather than only
        // on meaningful velocity change -- unlike ItemPhysicsSystem, there's
        // no lengthy horizontal arc for the client to extrapolate here.
        int entityId = entity.entityId;
        double px = entity.x, pz = entity.z;
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, px, newY, pz](int threshold) {
            return std::make_shared<Teleport_Entity_p>(threshold, entityId, px, newY, pz, 0.0f, 0.0f, false);
        });
    }
}

string FallingBlockSystem::getName() const {
    return "FallingBlock";
}
