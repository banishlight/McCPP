#include <generators/FlatChunkGenerator.hpp>

std::shared_ptr<Chunk> FlatChunkGenerator::generate(int chunkX, int chunkZ) {
    auto chunk = std::make_shared<Chunk>(chunkX, chunkZ);
    chunk->setSectionBlock(GROUND_SECTION, GROUND_BLOCK_STATE_ID);
    // Every other section stays air (Chunk's default).
    return chunk;
}

string FlatChunkGenerator::getName() const {
    return "Flat";
}