#pragma once
#include <ChunkGenerator.hpp>
#include <PerlinNoise.hpp>
#include <BlockIds.hpp>
#include <array>
#include <vector>
#include <utility>

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
        // Called once per column right after its blocks are placed (reusing
        // the already-known surface height/block -- no second height scan).
        // Only ever grows something on a grass surface. `x`/`z` are chunk-
        // local (0-15); `worldX`/`worldZ`/`surfaceWorldY` are absolute.
        //
        // IMPORTANT: generate() builds one isolated Chunk per call with no
        // reference to any neighbor, and neighboring chunks can generate
        // concurrently on different WorldWorkerPool threads in no guaranteed
        // order (a neighbor may never generate at all). Chunk::setBlock's
        // local x/z is NOT bounds-checked -- writing outside 0-15 is
        // undefined behavior that silently corrupts this same chunk's own
        // arrays, never an actual neighbor. So a tree's full canopy must be
        // guaranteed to stay within local 0-15, which is why tree placement
        // is additionally gated on the TREE_CANOPY_RADIUS margin check below,
        // on top of placeLeafLayer's own defensive bounds check.
        //
        // `treePositions` accumulates every tree already placed so far in
        // this chunk (chunk-local x/z) -- each new candidate is rejected if
        // it's within MIN_TREE_SPACING of an existing one, so two adjacent
        // columns can't both independently roll a tree and produce two
        // overlapping, "doubled-up" canopies (a real reported bug: nothing
        // in the per-column roll on its own prevents neighboring columns
        // from both crossing the threshold, since gradient noise is smooth
        // enough that nearby columns' rolls are correlated).
        void decorateColumn(std::shared_ptr<Chunk>& chunk, int x, int z, int worldX, int worldZ, int surfaceWorldY, Int32 surfaceBlock, Biome biome, std::vector<std::pair<int, int>>& treePositions) const;
        // `roll` is the same vegetation-noise sample decorateColumn already
        // computed -- reused to vary trunk height so no extra noise
        // evaluation is needed for that.
        void placeTree(const std::shared_ptr<Chunk>& chunk, int x, int z, int surfaceWorldY, double roll) const;
        // One layer of leaves in a (2*radius+1) square centered on (x,z) at
        // height y, skipping the 4 corners when omitCorners is set (a
        // rounded look) and skipping any position that isn't currently air
        // (never overwrites the trunk, or any terrain poking into the
        // canopy's space).
        void placeLeafLayer(const std::shared_ptr<Chunk>& chunk, int x, int z, int y, int radius, bool omitCorners) const;

        PerlinNoise _terrainNoise;
        PerlinNoise _caveNoise;
        PerlinNoise _temperatureNoise;
        PerlinNoise _humidityNoise;
        PerlinNoise _vegetationNoise;

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

        // Vegetation placement (tunable). Frequency is much higher than
        // climate's -- vegetation should scatter as individual points, not
        // smooth clustered regions, so a single octave at near-white-noise
        // frequency is used deliberately instead of the fbm layering
        // terrain/climate use. Thresholds are calibrated against this single
        // octave's real practical output range (empirically about
        // [-0.8, 0.8], not the full theoretical [-1, 1]) rather than assumed.
        static constexpr double VEGETATION_FREQUENCY = 1.0 / 3.0;
        static constexpr double TREE_ROLL_THRESHOLD = 0.5;   // roll above this -> tree attempt (Plains + margin permitting), ~1% of columns
        static constexpr double PLANT_ROLL_THRESHOLD = -0.3; // roll below this -> a single ground plant, ~11% of grass columns
        static constexpr int TREE_CANOPY_RADIUS = 2;         // max leaf-layer radius -- also the required chunk-edge margin
        static constexpr int TREE_MIN_HEIGHT = 4;
        static constexpr int TREE_MAX_HEIGHT = 6;
        static constexpr int MIN_TREE_SPACING = 5;           // minimum x/z separation between two trees in the same chunk
};