#pragma once
#include <Standards.hpp>
#include <array>

// One chunk column's data. Every section is uniform (a single block-state ID
// for the whole 16x16x16 section) rather than a full per-block palette --
// this reuses the existing single-valued wire encoding entirely; a real
// terrain generator with mixed-block sections will need a proper indirect
// palette encoder later.
class Chunk {
    public:
        static constexpr int WORLD_HEIGHT = 384;
        static constexpr int SECTION_COUNT = WORLD_HEIGHT / 16; // 24

        Chunk(int chunkX, int chunkZ);
        int getChunkX() const;
        int getChunkZ() const;
        void setSectionBlock(int sectionIndex, Int32 blockStateId);
        Int32 getSectionBlock(int sectionIndex) const;
        Int32 getBiomeId() const;
    private:
        int _chunkX;
        int _chunkZ;
        std::array<Int32, SECTION_COUNT> _sectionBlocks{}; // defaults to 0 (air)
        Int32 _biomeId = 0;
};