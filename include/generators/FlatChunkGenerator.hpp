#pragma once
#include <ChunkGenerator.hpp>

// Simplest possible world: the bottom section is solid stone, everything
// above is air. Not vanilla-accurate (real flat worlds layer bedrock/dirt/
// dirt/grass_block within one section, which needs a bit-packed indirect
// palette this generator deliberately doesn't build yet) -- this proves out
// the ChunkGenerator/multi-chunk mechanism with a still-uniform-per-section
// chunk, reusing the existing single-valued wire encoding.
class FlatChunkGenerator : public ChunkGenerator {
    public:
        std::shared_ptr<Chunk> generate(int chunkX, int chunkZ) override;
        string getName() const override;
    private:
        static constexpr int GROUND_SECTION = 0; // y=-64..-49
        static constexpr Int32 GROUND_BLOCK_STATE_ID = 1; // minecraft:stone (verified via server.jar --reports data)
};