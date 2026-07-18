#include <World.hpp>
#include <generators/FlatChunkGenerator.hpp>

World::World() {
    // Only one generator exists today; _isFlat is the selection point for
    // when a real terrain generator exists to choose between.
    if (_isFlat) {
        _generator = std::make_unique<FlatChunkGenerator>();
    }
}

World& World::getInstance() {
    static World instance;
    return instance;
}

string World::getDimensionName() const {
    return _dimensionName;
}

double World::getSpawnX() const {
    return _spawnX;
}

double World::getSpawnY() const {
    return _spawnY;
}

double World::getSpawnZ() const {
    return _spawnZ;
}

float World::getSpawnYaw() const {
    return _spawnYaw;
}

Int64 World::getHashedSeed() const {
    return _hashedSeed;
}

bool World::isFlat() const {
    return _isFlat;
}

std::shared_ptr<Chunk> World::getChunk(int chunkX, int chunkZ) {
    std::lock_guard<std::mutex> lock(_chunkCacheMutex);
    auto key = std::make_pair(chunkX, chunkZ);
    auto it = _chunkCache.find(key);
    if (it != _chunkCache.end()) {
        return it->second;
    }
    std::shared_ptr<Chunk> chunk = _generator->generate(chunkX, chunkZ);
    _chunkCache[key] = chunk;
    return chunk;
}