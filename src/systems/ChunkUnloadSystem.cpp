#include <systems/ChunkUnloadSystem.hpp>
#include <World.hpp>

void ChunkUnloadSystem::onTick(Int64 tickCount) {
    if (tickCount % SWEEP_INTERVAL_TICKS != 0) return;
    World::getInstance().evictStaleChunks(GRACE_PERIOD_SECONDS);
    World::getInstance().evictStaleTerrainCache();
}

string ChunkUnloadSystem::getName() const {
    return "ChunkUnload";
}
