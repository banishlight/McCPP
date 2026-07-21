#include <Chunk.hpp>

int floorDiv16(int v) {
    return (v >= 0) ? (v / 16) : ((v - 15) / 16);
}

Chunk::Chunk(int chunkX, int chunkZ) : _chunkX(chunkX), _chunkZ(chunkZ) {
}

int Chunk::getChunkX() const {
    return _chunkX;
}

int Chunk::getChunkZ() const {
    return _chunkZ;
}

int Chunk::index(int localX, int worldY, int localZ) const {
    // (y*256 + z*16 + x) over the full 0-383 local-Y range decomposes exactly
    // into section*4096 + in-section offset (256 * 16 == 4096), so no separate
    // section/within-section split is needed here.
    int localY = worldY - WORLD_MIN_Y;
    return localY * 256 + localZ * 16 + localX;
}

void Chunk::setBlock(int localX, int worldY, int localZ, Int32 blockStateId) {
    _blocks[index(localX, worldY, localZ)] = static_cast<Int16>(blockStateId);
}

Int32 Chunk::getBlock(int localX, int worldY, int localZ) const {
    return static_cast<Int32>(_blocks[index(localX, worldY, localZ)]);
}

void Chunk::setSkyLight(int localX, int worldY, int localZ, int level) {
    _skyLight[index(localX, worldY, localZ)] = static_cast<uint8_t>(level);
}

int Chunk::getSkyLight(int localX, int worldY, int localZ) const {
    return _skyLight[index(localX, worldY, localZ)];
}

void Chunk::setBlockLight(int localX, int worldY, int localZ, int level) {
    _blockLight[index(localX, worldY, localZ)] = static_cast<uint8_t>(level);
}

int Chunk::getBlockLight(int localX, int worldY, int localZ) const {
    return _blockLight[index(localX, worldY, localZ)];
}

Int32 Chunk::getBiomeId() const {
    return _biomeId;
}

void Chunk::setBiomeId(Int32 biomeId) {
    _biomeId = biomeId;
}