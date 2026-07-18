#include <World.hpp>
#include <generators/FlatChunkGenerator.hpp>
#include <generators/NoiseChunkGenerator.hpp>
#include <WorldWorkerPool.hpp>
#include <LightEngine.hpp>

World::World() {
    // Only two generators exist today; _isFlat is the selection point for
    // when more real terrain generators exist to choose between.
    if (_isFlat) {
        _generator = std::make_unique<FlatChunkGenerator>();
    } else {
        _generator = std::make_unique<NoiseChunkGenerator>(_hashedSeed);

        // There's no fixed safe spawn height once terrain isn't flat (even
        // "solid at (0,0)" could be inside a cave void), so generate the spawn
        // column once at startup and scan it top-down for the first solid
        // block with air directly above it -- mirrors vanilla's spawn-safety
        // concept, simplified to a single column since this is a dev server.
        std::shared_ptr<Chunk> spawnTerrain = getOrGenerateTerrain(0, 0);
        const Int32 AIR_BLOCK_STATE_ID = 0;
        for (int y = Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 2; y >= Chunk::WORLD_MIN_Y; y--) {
            if (spawnTerrain->getBlock(0, y, 0) != AIR_BLOCK_STATE_ID && spawnTerrain->getBlock(0, y + 1, 0) == AIR_BLOCK_STATE_ID) {
                _spawnY = y + 1.0;
                break;
            }
        }
        // A chunk only ever enters _chunkCache once its lighting is fully
        // computed -- see getOrGenerateTerrain/getChunkAsync. Easy to miss
        // for this one since it's cached directly here rather than through
        // getChunkAsync, but skipping it would leave the single most-visited
        // chunk in the game permanently dark. Lighting is computed into a
        // private copy, never into the shared terrain-cache object itself
        // (that object may be concurrently read by other threads as a
        // lighting neighbor and must never be mutated after generation).
        std::shared_ptr<Chunk> spawnChunk = std::make_shared<Chunk>(*spawnTerrain);
        LightEngine::computeLighting(*spawnChunk, *this);
        _chunkCache[{0, 0}] = spawnChunk;
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

void World::getChunkAsync(int chunkX, int chunkZ, std::function<void(std::shared_ptr<Chunk>)> callback) {
    std::shared_ptr<Chunk> cached;
    {
        std::lock_guard<std::mutex> lock(_chunkCacheMutex);
        auto it = _chunkCache.find({chunkX, chunkZ});
        if (it != _chunkCache.end()) {
            cached = it->second;
        }
    }
    if (cached) {
        callback(cached);
        return;
    }
    WorldWorkerPool::getInstance().submit([this, chunkX, chunkZ, callback]() {
        std::shared_ptr<Chunk> terrain = getOrGenerateTerrain(chunkX, chunkZ);
        // Copy before lighting: `terrain` may be concurrently shared with
        // other threads reading it as a lighting neighbor and must never be
        // mutated after generation.
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(*terrain);
        LightEngine::computeLighting(*chunk, *this);
        {
            std::lock_guard<std::mutex> lock(_chunkCacheMutex);
            // Last-write-wins if two requests raced to light the same
            // never-before-cached column concurrently -- harmless, since
            // lighting is a pure function of the (already-cached) terrain.
            _chunkCache[{chunkX, chunkZ}] = chunk;
        }
        callback(chunk);
    });
}

std::shared_ptr<Chunk> World::getOrGenerateTerrain(int chunkX, int chunkZ) {
    {
        std::lock_guard<std::mutex> lock(_terrainCacheMutex);
        auto it = _terrainCache.find({chunkX, chunkZ});
        if (it != _terrainCache.end()) {
            return it->second;
        }
    }
    std::shared_ptr<Chunk> chunk = _generator->generate(chunkX, chunkZ);
    {
        std::lock_guard<std::mutex> lock(_terrainCacheMutex);
        // Last-write-wins if two threads raced to generate the same
        // never-before-cached column concurrently -- harmless (deterministic,
        // pure function of chunkX/chunkZ/seed), and rare in practice.
        _terrainCache[{chunkX, chunkZ}] = chunk;
    }
    return chunk;
}