#include <generators/NoiseChunkGenerator.hpp>
#include <Chunk.hpp>

NoiseChunkGenerator::NoiseChunkGenerator(Int64 seed)
    : _terrainNoise(seed), _caveNoise(seed ^ static_cast<Int64>(0x9E3779B97F4A7C15ULL)) {
}

bool NoiseChunkGenerator::isCave(int worldX, int worldY, int worldZ) const {
    if (worldY >= static_cast<int>(CAVE_CEILING_Y) || worldY <= Chunk::WORLD_MIN_Y + 4) {
        return false; // stay clear of the surface band and the world floor
    }
    double n = _caveNoise.fbm(worldX * CAVE_FREQUENCY, worldY * CAVE_FREQUENCY, worldZ * CAVE_FREQUENCY,
                               CAVE_OCTAVES, CAVE_PERSISTENCE, CAVE_LACUNARITY);
    return n > CAVE_THRESHOLD;
}

bool NoiseChunkGenerator::isSolid(int worldX, int worldY, int worldZ) const {
    double n = _terrainNoise.fbm(worldX * TERRAIN_FREQUENCY, worldY * TERRAIN_FREQUENCY * TERRAIN_VERTICAL_SCALE, worldZ * TERRAIN_FREQUENCY,
                                  TERRAIN_OCTAVES, TERRAIN_PERSISTENCE, TERRAIN_LACUNARITY);
    double bias = (worldY - SURFACE_LEVEL_Y) / HEIGHT_BAND_HALF_WIDTH;
    double density = n - bias;
    if (density <= 0.0) return false;
    if (isCave(worldX, worldY, worldZ)) return false;
    return true;
}

Int32 NoiseChunkGenerator::pickBlock(int worldX, int worldY, int worldZ) const {
    for (int depth = 1; depth <= DIRT_DEPTH + 1; depth++) {
        if (!isSolid(worldX, worldY + depth, worldZ)) {
            return depth == 1 ? GRASS_BLOCK_STATE_ID : DIRT_BLOCK_STATE_ID;
        }
    }
    return STONE_BLOCK_STATE_ID;
}

std::shared_ptr<Chunk> NoiseChunkGenerator::generate(int chunkX, int chunkZ) {
    auto chunk = std::make_shared<Chunk>(chunkX, chunkZ);
    int baseX = chunkX * 16;
    int baseZ = chunkZ * 16;
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int worldX = baseX + x;
            int worldZ = baseZ + z;
            for (int y = 0; y < Chunk::WORLD_HEIGHT; y++) {
                int worldY = Chunk::WORLD_MIN_Y + y;
                if (isSolid(worldX, worldY, worldZ)) {
                    chunk->setBlock(x, worldY, z, pickBlock(worldX, worldY, worldZ));
                }
                // else leave as air (Chunk's default).
            }
        }
    }
    return chunk;
}

string NoiseChunkGenerator::getName() const {
    return "Noise";
}