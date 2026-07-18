#pragma once
#include <ChunkGenerator.hpp>
#include <PerlinNoise.hpp>

// Real terrain: a 3D density field (terrain noise minus a height bias, so the
// world trends solid low / air high, crossing zero in a rolling "surface
// band") with a second noise pass carving simple caves below that band.
// Surface blocks are picked by walking upward through the same density
// function -- no neighbor-chunk data needed, since it's pure in world space.
class NoiseChunkGenerator : public ChunkGenerator {
    public:
        explicit NoiseChunkGenerator(Int64 seed);
        std::shared_ptr<Chunk> generate(int chunkX, int chunkZ) override;
        string getName() const override;
    private:
        bool isSolid(int worldX, int worldY, int worldZ) const;
        bool isCave(int worldX, int worldY, int worldZ) const;
        Int32 pickBlock(int worldX, int worldY, int worldZ) const;

        PerlinNoise _terrainNoise;
        PerlinNoise _caveNoise;

        // Sourced from the vanilla data generator report (server.jar --reports), not guessed.
        static constexpr Int32 STONE_BLOCK_STATE_ID = 1;
        static constexpr Int32 DIRT_BLOCK_STATE_ID = 10;
        static constexpr Int32 GRASS_BLOCK_STATE_ID = 9; // snowy=false (default)
        static constexpr int DIRT_DEPTH = 3;

        // Terrain shape (tunable; iterate by playtesting).
        static constexpr double TERRAIN_FREQUENCY = 1.0 / 96.0;
        static constexpr double TERRAIN_VERTICAL_SCALE = 2.0; // squashes Y so hills aren't blobby
        static constexpr int TERRAIN_OCTAVES = 4;
        static constexpr double TERRAIN_PERSISTENCE = 0.5;
        static constexpr double TERRAIN_LACUNARITY = 2.0;
        static constexpr double SURFACE_LEVEL_Y = 24.0;
        static constexpr double HEIGHT_BAND_HALF_WIDTH = 40.0;

        // Cave shape (tunable).
        static constexpr double CAVE_FREQUENCY = 1.0 / 20.0;
        static constexpr int CAVE_OCTAVES = 2;
        static constexpr double CAVE_PERSISTENCE = 0.5;
        static constexpr double CAVE_LACUNARITY = 2.0;
        static constexpr double CAVE_THRESHOLD = 0.65;
        static constexpr double CAVE_CEILING_Y = SURFACE_LEVEL_Y - 8.0; // stay well below the surface band
};