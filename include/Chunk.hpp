#pragma once
#include <Standards.hpp>
#include <array>

// Floor division by 16 (toward negative infinity), unlike C++'s truncating /
// -- needed anywhere a world block coordinate maps to a chunk coordinate.
int floorDiv16(int v);

// One chunk column's data: a real per-block store (not just one ID per
// section) since terrain height varies per column, so sections are no longer
// uniform the way they were for the flat world.
class Chunk {
    public:
        static constexpr int WORLD_MIN_Y = -64;
        static constexpr int WORLD_HEIGHT = 384;
        static constexpr int SECTION_COUNT = WORLD_HEIGHT / 16; // 24

        Chunk(int chunkX, int chunkZ);
        int getChunkX() const;
        int getChunkZ() const;

        // localX/localZ: 0-15, chunk-relative. worldY: WORLD_MIN_Y..WORLD_MIN_Y+WORLD_HEIGHT-1,
        // absolute world height -- callers evaluating a density function need
        // world coordinates for continuity across chunk boundaries anyway, so
        // this avoids a separate local/world Y conversion at every call site.
        void setBlock(int localX, int worldY, int localZ, Int32 blockStateId);
        Int32 getBlock(int localX, int worldY, int localZ) const;

        // Light levels 0-15. Only ever meaningful once LightEngine::computeLighting
        // has run -- see World's "a chunk only enters the cache once lit" invariant.
        void setSkyLight(int localX, int worldY, int localZ, int level);
        int getSkyLight(int localX, int worldY, int localZ) const;
        void setBlockLight(int localX, int worldY, int localZ, int level);
        int getBlockLight(int localX, int worldY, int localZ) const;

        Int32 getBiomeId() const;
    private:
        int index(int localX, int worldY, int localZ) const;
        int _chunkX;
        int _chunkZ;
        std::array<Int32, SECTION_COUNT * 4096> _blocks{}; // defaults to 0 (air)
        std::array<uint8_t, SECTION_COUNT * 4096> _skyLight{};
        std::array<uint8_t, SECTION_COUNT * 4096> _blockLight{};
        Int32 _biomeId = 0;
};