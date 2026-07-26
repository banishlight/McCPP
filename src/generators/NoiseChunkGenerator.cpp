#include <generators/NoiseChunkGenerator.hpp>
#include <Chunk.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <cstdlib>

namespace {
    const char* biomeToName(Biome biome) {
        switch (biome) {
            case Biome::Desert: return "minecraft:desert";
            case Biome::SnowyPlains: return "minecraft:snowy_plains";
            case Biome::Savanna: return "minecraft:savanna";
            case Biome::Plains: default: return "minecraft:plains";
        }
    }

    // Biome registry index lookup -- same "send order isn't fixed, look up
    // by id" convention already used for dimension_type/chat_type/biome
    // elsewhere in this project (ChunkNbtCodec has its own near-identical
    // copy for the disk-import path; kept separate rather than shared, since
    // each is only a few lines).
    int biomeNameToIndex(const string& name) {
        const std::vector<RegistryEntry>& biomes = VanillaDataManager::getInstance().getEntries("worldgen/biome");
        for (size_t i = 0; i < biomes.size(); i++) {
            if (biomes[i].id == name) return static_cast<int>(i);
        }
        return 0;
    }
}

NoiseChunkGenerator::NoiseChunkGenerator(Int64 seed)
    : _terrainNoise(seed), _caveNoise(seed ^ static_cast<Int64>(0x9E3779B97F4A7C15ULL)),
      _temperatureNoise(seed ^ static_cast<Int64>(0xC2B2AE3D27D4EB4FULL)),
      _humidityNoise(seed ^ static_cast<Int64>(0x165667B19E3779F9ULL)),
      _vegetationNoise(seed ^ static_cast<Int64>(0x94D049BB133111EBULL)) {
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

Biome NoiseChunkGenerator::selectBiome(int chunkX, int chunkZ) const {
    double worldX = chunkX * 16 + 8;
    double worldZ = chunkZ * 16 + 8;
    double temperature = _temperatureNoise.fbm(worldX * CLIMATE_FREQUENCY, 0.0, worldZ * CLIMATE_FREQUENCY,
                                                CLIMATE_OCTAVES, CLIMATE_PERSISTENCE, CLIMATE_LACUNARITY);
    if (temperature < COLD_THRESHOLD) return Biome::SnowyPlains;
    if (temperature > HOT_THRESHOLD) {
        double humidity = _humidityNoise.fbm(worldX * CLIMATE_FREQUENCY, 0.0, worldZ * CLIMATE_FREQUENCY,
                                              CLIMATE_OCTAVES, CLIMATE_PERSISTENCE, CLIMATE_LACUNARITY);
        return humidity < DRY_HUMIDITY_THRESHOLD ? Biome::Desert : Biome::Savanna;
    }
    return Biome::Plains;
}

Int32 NoiseChunkGenerator::pickBlockFromColumn(const std::array<bool, Chunk::WORLD_HEIGHT>& column, int y, Biome biome) const {
    for (int depth = 1; depth <= DIRT_DEPTH + 1; depth++) {
        int ny = y + depth;
        bool solidAbove = (ny < Chunk::WORLD_HEIGHT) && column[ny]; // above world = air
        if (!solidAbove) {
            if (biome == Biome::Desert) return SAND_BLOCK_STATE_ID;
            if (depth == 1) return biome == Biome::SnowyPlains ? SNOW_BLOCK_STATE_ID : GRASS_BLOCK_STATE_ID;
            return DIRT_BLOCK_STATE_ID;
        }
    }
    return STONE_BLOCK_STATE_ID;
}

void NoiseChunkGenerator::placeLeafLayer(const std::shared_ptr<Chunk>& chunk, int x, int z, int y, int radius, bool omitCorners) const {
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dz = -radius; dz <= radius; dz++) {
            if (omitCorners && std::abs(dx) == radius && std::abs(dz) == radius) continue;
            int lx = x + dx, lz = z + dz;
            if (lx < 0 || lx > 15 || lz < 0 || lz > 15) continue; // defensive -- see decorateColumn's comment
            if (chunk->getBlock(lx, y, lz) != AIR_BLOCK_STATE_ID) continue; // never overwrite the trunk (or stray terrain)
            chunk->setBlock(lx, y, lz, OAK_LEAVES_STATE_ID);
        }
    }
}

void NoiseChunkGenerator::placeTree(const std::shared_ptr<Chunk>& chunk, int x, int z, int surfaceWorldY, double roll) const {
    double t = (roll - TREE_ROLL_THRESHOLD) / (1.0 - TREE_ROLL_THRESHOLD); // 0..1
    int height = TREE_MIN_HEIGHT + static_cast<int>(t * (TREE_MAX_HEIGHT - TREE_MIN_HEIGHT + 1));
    if (height > TREE_MAX_HEIGHT) height = TREE_MAX_HEIGHT;

    int trunkBaseY = surfaceWorldY + 1;
    for (int i = 0; i < height; i++) {
        chunk->setBlock(x, trunkBaseY + i, z, OAK_LOG_STATE_ID);
    }
    int topY = trunkBaseY + height - 1;
    placeLeafLayer(chunk, x, z, topY - 2, 2, true); // 5x5 minus corners
    placeLeafLayer(chunk, x, z, topY - 1, 2, true); // 5x5 minus corners
    placeLeafLayer(chunk, x, z, topY,     1, true); // 3x3 minus corners
    placeLeafLayer(chunk, x, z, topY + 1, 1, true); // plus-shaped cap
}

void NoiseChunkGenerator::decorateColumn(std::shared_ptr<Chunk>& chunk, int x, int z, int worldX, int worldZ, int surfaceWorldY, Int32 surfaceBlock, Biome biome, std::vector<std::pair<int, int>>& treePositions) const {
    if (surfaceBlock != GRASS_BLOCK_STATE_ID) return; // only grassy surfaces grow anything in v1

    double roll = _vegetationNoise.noise(worldX * VEGETATION_FREQUENCY, 0.0, worldZ * VEGETATION_FREQUENCY);

    bool marginOk = x >= TREE_CANOPY_RADIUS && x <= 15 - TREE_CANOPY_RADIUS
                 && z >= TREE_CANOPY_RADIUS && z <= 15 - TREE_CANOPY_RADIUS;
    if (biome == Biome::Plains && marginOk && roll > TREE_ROLL_THRESHOLD) {
        bool tooClose = false;
        for (auto& [tx, tz] : treePositions) {
            if (std::abs(x - tx) < MIN_TREE_SPACING && std::abs(z - tz) < MIN_TREE_SPACING) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            placeTree(chunk, x, z, surfaceWorldY, roll);
            treePositions.push_back({x, z});
        }
        return;
    }

    if (roll < PLANT_ROLL_THRESHOLD) {
        Int32 plant = SHORT_GRASS_STATE_ID;
        if (roll < -0.6) plant = POPPY_STATE_ID;
        else if (roll < -0.5) plant = DANDELION_STATE_ID;
        chunk->setBlock(x, surfaceWorldY + 1, z, plant);
    }
}

std::shared_ptr<Chunk> NoiseChunkGenerator::generate(int chunkX, int chunkZ) {
    auto chunk = std::make_shared<Chunk>(chunkX, chunkZ);
    int baseX = chunkX * 16;
    int baseZ = chunkZ * 16;
    // Biome is resolved once per chunk (climate varies far more slowly than
    // terrain shape -- see selectBiome), not per-column.
    Biome biome = selectBiome(chunkX, chunkZ);
    chunk->setBiomeId(biomeNameToIndex(biomeToName(biome)));
    // Compute isSolid() once per (x,y,z) into a per-column buffer, then reuse
    // it for both this block's own placement and any other block's upward
    // exposure check (pickBlockFromColumn) -- avoids up to DIRT_DEPTH+1
    // redundant noise evaluations per solid block, which dominated generation
    // time (each isSolid() call is several fbm octave evaluations).
    std::array<bool, Chunk::WORLD_HEIGHT> column{};
    std::vector<std::pair<int, int>> treePositions; // see decorateColumn -- enforces MIN_TREE_SPACING
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int worldX = baseX + x;
            int worldZ = baseZ + z;
            for (int y = 0; y < Chunk::WORLD_HEIGHT; y++) {
                column[y] = isSolid(worldX, Chunk::WORLD_MIN_Y + y, worldZ);
            }
            int surfaceLocalY = -1;
            Int32 surfaceBlock = AIR_BLOCK_STATE_ID;
            for (int y = 0; y < Chunk::WORLD_HEIGHT; y++) {
                if (column[y]) {
                    surfaceBlock = pickBlockFromColumn(column, y, biome);
                    chunk->setBlock(x, Chunk::WORLD_MIN_Y + y, z, surfaceBlock);
                    surfaceLocalY = y; // ascending loop -- last write is the topmost solid block
                }
                // else leave as air (Chunk's default).
            }
            if (surfaceLocalY >= 0) {
                decorateColumn(chunk, x, z, worldX, worldZ, Chunk::WORLD_MIN_Y + surfaceLocalY, surfaceBlock, biome, treePositions);
            }
        }
    }
    return chunk;
}

string NoiseChunkGenerator::getName() const {
    return "Noise";
}