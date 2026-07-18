#include <Chunk.hpp>

Chunk::Chunk(int chunkX, int chunkZ) : _chunkX(chunkX), _chunkZ(chunkZ) {
}

int Chunk::getChunkX() const {
    return _chunkX;
}

int Chunk::getChunkZ() const {
    return _chunkZ;
}

void Chunk::setSectionBlock(int sectionIndex, Int32 blockStateId) {
    _sectionBlocks[sectionIndex] = blockStateId;
}

Int32 Chunk::getSectionBlock(int sectionIndex) const {
    return _sectionBlocks[sectionIndex];
}

Int32 Chunk::getBiomeId() const {
    return _biomeId;
}