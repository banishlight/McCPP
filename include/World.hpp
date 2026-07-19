#pragma once
#include <Standards.hpp>
#include <Chunk.hpp>
#include <ChunkGenerator.hpp>
#include <map>
#include <mutex>
#include <memory>
#include <functional>

// Exists as the query boundary a real multi-chunk world sits behind, so
// callers ask "the world" for spawn/chunk data instead of hardcoding it inline.
class World {
    public:
        static World& getInstance();
        string getDimensionName() const;
        double getSpawnX() const;
        double getSpawnY() const;
        double getSpawnZ() const;
        float getSpawnYaw() const;
        Int64 getHashedSeed() const;
        bool isFlat() const;
        // Resolves the chunk for (chunkX, chunkZ): a cache hit invokes callback
        // immediately on the caller's thread; a cache miss generates it on
        // WorldWorkerPool and invokes callback from a pool thread once ready.
        // Safe to call from any thread.
        void getChunkAsync(int chunkX, int chunkZ, std::function<void(std::shared_ptr<Chunk>)> callback);
        // Terrain-only, cached, safe for concurrent reads (a chunk's block
        // data is never mutated after generation). Used both by LightEngine's
        // neighbor queries and internally by getChunkAsync, so a chunk's
        // terrain is only ever generated once no matter how many nearby
        // chunks need it purely for lighting occlusion. Safe from any thread.
        std::shared_ptr<Chunk> getOrGenerateTerrain(int chunkX, int chunkZ);
        // Synchronous read of an already-cached, fully-lit chunk -- nullptr if
        // not cached (not loaded/generated yet). Safe from any thread.
        std::shared_ptr<Chunk> getCachedChunk(int chunkX, int chunkZ);
        // Applies one block edit at absolute world coordinates. Copy-then-replace
        // (see docs/general-documentation.md, "Terrain cache vs. lit cache"):
        // builds a modified copy of the cached chunk, relights just that copy,
        // then swaps it into _chunkCache -- the previous shared_ptr, if any
        // other thread is mid-read of it, is left untouched and internally
        // consistent. _terrainCache is intentionally not updated (accepted
        // gap: a neighbor chunk relit later than this edit sees stale terrain
        // for this chunk). Returns false (no-op) if the chunk isn't cached.
        bool setBlock(int worldX, int worldY, int worldZ, Int32 blockStateId);
    private:
        World();
        string _dimensionName = "minecraft:overworld";
        double _spawnX = 0.5;
        double _spawnY = -48.0; // placeholder; recomputed in the constructor for non-flat generators
        double _spawnZ = 0.5;
        float _spawnYaw = 0.0f;
        Int64 _hashedSeed = 0;
        bool _isFlat = false;
        std::unique_ptr<ChunkGenerator> _generator;
        std::map<std::pair<int,int>, std::shared_ptr<Chunk>> _chunkCache; // fully lit, delivered to players
        std::mutex _chunkCacheMutex;
        std::map<std::pair<int,int>, std::shared_ptr<Chunk>> _terrainCache; // block data only, no light
        std::mutex _terrainCacheMutex;
};