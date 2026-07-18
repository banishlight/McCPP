#include <Chunk.hpp>

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
    _blocks[index(localX, worldY, localZ)] = blockStateId;
}

Int32 Chunk::getBlock(int localX, int worldY, int localZ) const {
    return _blocks[index(localX, worldY, localZ)];
}

Int32 Chunk::getBiomeId() const {
    return _biomeId;
}