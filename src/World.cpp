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
        // Light into a private copy, never the shared terrain-cache object
        // itself (other threads may be concurrently reading it as a lighting
        // neighbor) -- see docs/general-documentation.md, "Terrain cache vs. lit cache".
        std::shared_ptr<Chunk> spawnChunk = std::make_shared<Chunk>(*spawnTerrain);
        LightEngine::computeLighting(*spawnChunk, *this);
        cacheAsLit(0, 0, spawnChunk);
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
    std::shared_ptr<Chunk> cached = getCachedChunk(chunkX, chunkZ);
    if (cached) {
        callback(cached);
        return;
    }
    WorldWorkerPool::getInstance().submit([this, chunkX, chunkZ, callback]() {
        std::shared_ptr<Chunk> terrain = getOrGenerateTerrain(chunkX, chunkZ);
        // Copy before lighting -- `terrain` may be concurrently shared as a
        // lighting neighbor and must never be mutated after generation.
        std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(*terrain);
        LightEngine::computeLighting(*chunk, *this);
        // Last-write-wins on a race is harmless: lighting is a pure function
        // of the (already-cached) terrain.
        cacheAsLit(chunkX, chunkZ, chunk);
        callback(chunk);
    });
}

std::shared_ptr<Chunk> World::getCachedChunk(int chunkX, int chunkZ) {
    std::lock_guard<std::mutex> lock(_chunkCacheMutex);
    auto it = _chunkCache.find({chunkX, chunkZ});
    return (it != _chunkCache.end()) ? it->second : nullptr;
}

void World::cacheAsLit(int chunkX, int chunkZ, std::shared_ptr<Chunk> chunk) {
    {
        std::lock_guard<std::mutex> lock(_chunkCacheMutex);
        _chunkCache[{chunkX, chunkZ}] = chunk;
    }
    std::lock_guard<std::mutex> lock(_terrainCacheMutex);
    _terrainCache.erase({chunkX, chunkZ});
}

bool World::setBlock(int worldX, int worldY, int worldZ, Int32 blockStateId) {
    int chunkX = floorDiv16(worldX);
    int chunkZ = floorDiv16(worldZ);
    std::shared_ptr<Chunk> cached = getCachedChunk(chunkX, chunkZ);
    if (!cached) return false;

    int localX = worldX - chunkX * 16;
    int localZ = worldZ - chunkZ * 16;
    std::shared_ptr<Chunk> edited = std::make_shared<Chunk>(*cached);
    edited->setBlock(localX, worldY, localZ, blockStateId);
    LightEngine::computeLighting(*edited, *this);
    cacheAsLit(chunkX, chunkZ, edited);
    return true;
}

std::shared_ptr<Chunk> World::getOrGenerateTerrain(int chunkX, int chunkZ) {
    // A fully-lit cached chunk already contains everything terrain-only data
    // would (and is more up to date if it's since been edited) -- check it
    // first so a chunk's storage is never duplicated across both caches.
    std::shared_ptr<Chunk> lit = getCachedChunk(chunkX, chunkZ);
    if (lit) return lit;

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
        // Last-write-wins on a race is harmless: generation is a pure
        // function of chunkX/chunkZ/seed.
        _terrainCache[{chunkX, chunkZ}] = chunk;
    }
    return chunk;
}