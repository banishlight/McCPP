#include <World.hpp>
#include <generators/FlatChunkGenerator.hpp>
#include <generators/NoiseChunkGenerator.hpp>
#include <WorldWorkerPool.hpp>
#include <LightEngine.hpp>
#include <LevelDat.hpp>
#include <WorldPersistence.hpp>
#include <VanillaVersion.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <random>
#include <cctype>

namespace {
    // Matches vanilla's actual level-seed handling (verified against
    // minecraft.wiki, not assumed): a string that parses fully as a signed
    // integer is used as the seed directly; anything else is hashed via
    // Java's String.hashCode() (32-bit signed, sign-extended into the 64-bit
    // seed field); an empty string gets a genuinely random seed, same as
    // vanilla does for a brand-new world with no configured seed.
    Int64 hashSeedString(const string& seedText) {
        if (seedText.empty()) {
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<Int64> dist;
            return dist(gen);
        }

        size_t start = (seedText[0] == '-' || seedText[0] == '+') ? 1 : 0;
        bool isNumeric = start < seedText.size();
        for (size_t i = start; i < seedText.size() && isNumeric; i++) {
            if (!std::isdigit(static_cast<unsigned char>(seedText[i]))) isNumeric = false;
        }
        if (isNumeric) {
            try {
                return static_cast<Int64>(std::stoll(seedText));
            } catch (...) {
                isNumeric = false; // out of range -- fall through to hashing, matching Java's NumberFormatException path
            }
        }

        // hash = 31*hash + charAt(i), computed with unsigned 32-bit
        // arithmetic (well-defined wraparound) then reinterpreted as signed
        // to avoid C++ signed-overflow UB while matching Java's wraparound int math.
        UInt32 hash = 0;
        for (unsigned char c : seedText) {
            hash = hash * 31u + static_cast<UInt32>(c);
        }
        return static_cast<Int64>(static_cast<Int32>(hash));
    }
}

World::World() {
    _worldDir = Properties::getProperties().level_name;
    // Must happen before anything below that could load a chunk (the spawn-
    // chunk priming a few lines down included) -- see WorldPersistence::initialize's
    // own comment for why this can't just read World::getInstance() itself.
    WorldPersistence::getInstance().initialize(_worldDir);
    std::optional<LevelData> loaded = LevelDat::tryLoad(_worldDir);
    // Governs which generator fills in anything not already covered by an
    // existing save (or all of a brand-new world) -- a server-operator
    // choice independent of whatever generator an imported save originally
    // used, which this project doesn't attempt to replicate.
    _isFlat = (Properties::getProperties().level_type == "minecraft:flat");

    if (loaded) {
        _hashedSeed = loaded->seed;
        _spawnX = loaded->spawnX;
        _spawnY = loaded->spawnY;
        _spawnZ = loaded->spawnZ;
        _spawnYaw = loaded->spawnYaw;
        Console::getConsole().Entry("World::World(): Loaded existing level.dat from '" + _worldDir + "' (seed " + std::to_string(_hashedSeed) + ").");
    } else {
        _hashedSeed = hashSeedString(Properties::getProperties().level_seed);
    }

    // Only two generators exist today; _isFlat is the selection point for
    // when more real terrain generators exist to choose between.
    if (_isFlat) {
        _generator = std::make_unique<FlatChunkGenerator>();
    } else {
        _generator = std::make_unique<NoiseChunkGenerator>(_hashedSeed);

        std::shared_ptr<Chunk> spawnTerrain = getOrGenerateTerrain(0, 0);
        if (!loaded) {
            // There's no fixed safe spawn height once terrain isn't flat (even
            // "solid at (0,0)" could be inside a cave void), so generate the spawn
            // column once at startup and scan it top-down for the first solid
            // block with air directly above it -- mirrors vanilla's spawn-safety
            // concept, simplified to a single column since this is a dev server.
            // Skipped when an existing level.dat already has a spawn point --
            // that's authoritative even if it doesn't look "safe" from here,
            // matching vanilla never re-deriving an existing world's spawn.
            const Int32 AIR_BLOCK_STATE_ID = 0;
            for (int y = Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 2; y >= Chunk::WORLD_MIN_Y; y--) {
                if (spawnTerrain->getBlock(0, y, 0) != AIR_BLOCK_STATE_ID && spawnTerrain->getBlock(0, y + 1, 0) == AIR_BLOCK_STATE_ID) {
                    _spawnY = y + 1.0;
                    break;
                }
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

const string& World::getWorldDir() const {
    return _worldDir;
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
        auto cleanIt = _cleanChunks.find({chunkX, chunkZ});
        if (cleanIt != _cleanChunks.end()) {
            _cleanChunks.erase(cleanIt); // just loaded from disk, unmodified -- not dirty
        } else {
            _dirtyChunks.insert({chunkX, chunkZ}); // freshly generated, or already dirty from setBlock
        }
    }
    std::lock_guard<std::mutex> lock(_terrainCacheMutex);
    _terrainCache.erase({chunkX, chunkZ});
}

std::vector<std::pair<int,int>> World::takeDirtyChunksSnapshot() {
    std::lock_guard<std::mutex> lock(_chunkCacheMutex);
    std::vector<std::pair<int,int>> result(_dirtyChunks.begin(), _dirtyChunks.end());
    _dirtyChunks.clear();
    return result;
}

LevelData World::buildLevelData() const {
    LevelData data;
    data.seed = _hashedSeed;
    data.levelName = _worldDir;
    data.spawnX = _spawnX;
    data.spawnY = _spawnY;
    data.spawnZ = _spawnZ;
    data.spawnYaw = _spawnYaw;
    data.dataVersion = VanillaVersion::CURRENT_DATA_VERSION;

    const string& gamemode = Properties::getProperties().gamemode;
    if (gamemode == "creative") data.gameType = 1;
    else if (gamemode == "adventure") data.gameType = 2;
    else if (gamemode == "spectator") data.gameType = 3;
    else data.gameType = 0; // survival

    data.difficulty = static_cast<int>(Properties::getProperties().difficulty);
    data.hardcore = Properties::getProperties().hardcore;
    data.dayTime = 0; // no day/night cycle system yet -- doesn't advance independently
    return data;
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
    std::shared_ptr<Chunk> chunk = WorldPersistence::getInstance().tryLoadChunk(chunkX, chunkZ);
    if (chunk) {
        // Loaded unmodified from disk -- the next cacheAsLit for this
        // coordinate shouldn't mark it dirty (see _cleanChunks).
        std::lock_guard<std::mutex> lock(_chunkCacheMutex);
        _cleanChunks.insert({chunkX, chunkZ});
    } else {
        chunk = _generator->generate(chunkX, chunkZ);
    }
    {
        std::lock_guard<std::mutex> lock(_terrainCacheMutex);
        // Last-write-wins on a race is harmless: generation is a pure
        // function of chunkX/chunkZ/seed, and a disk load is idempotent.
        _terrainCache[{chunkX, chunkZ}] = chunk;
    }
    return chunk;
}