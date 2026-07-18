#include <generators/FlatChunkGenerator.hpp>

std::shared_ptr<Chunk> FlatChunkGenerator::generate(int chunkX, int chunkZ) {
    auto chunk = std::make_shared<Chunk>(chunkX, chunkZ);
    int groundWorldY = Chunk::WORLD_MIN_Y + GROUND_SECTION * 16;
    for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                chunk->setBlock(x, groundWorldY + y, z, GROUND_BLOCK_STATE_ID);
            }
        }
    }
    // Every other section stays air (Chunk's default).
    return chunk;
}

string FlatChunkGenerator::getName() const {
    return "Flat";
}