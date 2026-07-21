#include <systems/AutosaveSystem.hpp>
#include <World.hpp>
#include <WorldPersistence.hpp>
#include <LevelDat.hpp>
#include <Console.hpp>

void AutosaveSystem::onTick(Int64 tickCount) {
    if (tickCount % AUTOSAVE_INTERVAL_TICKS != 0) return;
    saveNow();
}

void AutosaveSystem::saveNow() {
    World& world = World::getInstance();
    std::vector<std::pair<int,int>> dirty = world.takeDirtyChunksSnapshot();
    for (auto& [chunkX, chunkZ] : dirty) {
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) continue; // shouldn't happen, but never let a save crash over it
        WorldPersistence::getInstance().saveChunk(chunkX, chunkZ, *chunk);
    }
    LevelDat::save(world.getWorldDir(), world.buildLevelData());
    if (!dirty.empty()) {
        Console::getConsole().Entry("AutosaveSystem::saveNow(): Saved " + std::to_string(dirty.size()) + " chunk(s) and level.dat.");
    }
}

string AutosaveSystem::getName() const {
    return "Autosave";
}
