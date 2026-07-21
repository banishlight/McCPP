#include <WorldPersistence.hpp>
#include <ChunkNbtCodec.hpp>

WorldPersistence& WorldPersistence::getInstance() {
    static WorldPersistence instance;
    return instance;
}

void WorldPersistence::initialize(const string& worldDir) {
    std::lock_guard<std::mutex> lock(_mutex);
    _worldDir = worldDir;
}

std::shared_ptr<RegionFile> WorldPersistence::getOrOpenRegion(int regionX, int regionZ) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto key = std::make_pair(regionX, regionZ);
    auto it = _openRegions.find(key);
    if (it != _openRegions.end()) return it->second;

    auto region = std::make_shared<RegionFile>(_worldDir, regionX, regionZ);
    _openRegions[key] = region;
    return region;
}

std::shared_ptr<Chunk> WorldPersistence::tryLoadChunk(int chunkX, int chunkZ) {
    auto [regionX, regionZ] = RegionFile::regionCoordsFor(chunkX, chunkZ);
    std::shared_ptr<RegionFile> region = getOrOpenRegion(regionX, regionZ);

    int localX = chunkX & 31;
    int localZ = chunkZ & 31;
    std::optional<NbtTag> nbt = region->readChunk(localX, localZ);
    if (!nbt) return nullptr;

    std::shared_ptr<Chunk> chunk;
    ChunkDecodeResult result = ChunkNbtCodec::decodeChunk(*nbt, chunk);
    if (result != ChunkDecodeResult::Loaded) return nullptr; // not fully generated, unsupported version, or malformed -- fall back to generation
    return chunk;
}

void WorldPersistence::saveChunk(int chunkX, int chunkZ, const Chunk& chunk) {
    auto [regionX, regionZ] = RegionFile::regionCoordsFor(chunkX, chunkZ);
    std::shared_ptr<RegionFile> region = getOrOpenRegion(regionX, regionZ);

    int localX = chunkX & 31;
    int localZ = chunkZ & 31;
    NbtTag encoded = ChunkNbtCodec::encodeChunk(chunk, chunkX, chunkZ);
    region->writeChunk(localX, localZ, encoded);
}
