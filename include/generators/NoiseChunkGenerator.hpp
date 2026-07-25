#pragma once
#include <ChunkGenerator.hpp>
#include <PerlinNoise.hpp>
#include <BlockIds.hpp>
#include <array>

// Real terrain: a 3D density field (terrain noise minus a height bias, so the
// world trends solid low / air high, crossing zero in a rolling "surface
// band") with a second noise pass carving simple caves below that band.
// Surface blocks are picked by walking upward through the same density
// function -- no neighbor-chunk data needed, since it's pure in world space.
// One biome per whole chunk column (matches Chunk::_biomeId's existing
// single-value-per-column storage -- see docs), not vanilla's smoother
// per-4x4x4-cell resolution. A small, deliberately curated set: most other
// vanilla biomes' visual identity depends on trees/plants, which don't exist
// in this project yet. Savanna is included despite sharing Plains' surface
// blocks because a real client applies biome-correct grass/foliage tint
// automatically from the biome name alone, so it's already visually
// distinct with zero rendering work here.
enum class Biome {
    Plains,
    Desert,
    SnowyPlains,
    Savanna,
};

class NoiseChunkGenerator : public ChunkGenerator {
    public:
        explicit NoiseChunkGenerator(Int64 seed);
        std::shared_ptr<Chunk> generate(int chunkX, int chunkZ) override;
        string getName() const override;
    private:
        bool isSolid(int worldX, int worldY, int worldZ) const;
        bool isCave(int worldX, int worldY, int worldZ) const;
        // `column[y]` is the already-computed isSolid() result for this
        // (x,z) at local height `y` -- reused here instead of recomputing
        // (this is called for every solid block, so recomputation would mean
        // up to DIRT_DEPTH+1 redundant noise evaluations per solid block).
        Int32 pickBlockFromColumn(const std::array<bool, Chunk::WORLD_HEIGHT>& column, int y, Biome biome) const;
        // Sampled once per chunk (climate varies far more slowly than
        // terrain shape) at the chunk's center world coordinates -- original
        // noise thresholds, not vanilla's real multi-noise biome placement
        // algorithm (temperature/humidity/continentalness/erosion/weirdness),
        // consistent with this project's terrain shape already being its own
        // formula rather than a vanilla-accurate reproduction.
        Biome selectBiome(int chunkX, int chunkZ) const;

        PerlinNoise _terrainNoise;
        PerlinNoise _caveNoise;
        PerlinNoise _temperatureNoise;
        PerlinNoise _humidityNoise;

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

        // Climate shape (tunable). Frequency is much lower than terrain's --
        // biome regions should span hundreds of blocks, not tens.
        static constexpr double CLIMATE_FREQUENCY = 1.0 / 384.0;
        static constexpr int CLIMATE_OCTAVES = 2;
        static constexpr double CLIMATE_PERSISTENCE = 0.5;
        static constexpr double CLIMATE_LACUNARITY = 2.0;
        static constexpr double COLD_THRESHOLD = -0.15;   // temperature below this -> SnowyPlains
        static constexpr double HOT_THRESHOLD = 0.15;     // temperature above this -> Desert/Savanna
        static constexpr double DRY_HUMIDITY_THRESHOLD = 0.0; // within the hot band: below -> Desert, above -> Savanna
};