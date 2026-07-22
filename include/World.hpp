#pragma once
#include <Standards.hpp>
#include <Chunk.hpp>
#include <ChunkGenerator.hpp>
#include <LevelDat.hpp>
#include <map>
#include <set>
#include <mutex>
#include <memory>
#include <functional>
#include <vector>
#include <utility>
#include <chrono>
#include <atomic>

// Exists as the query boundary a real multi-chunk world sits behind, so
// callers ask "the world" for spawn/chunk data instead of hardcoding it inline.
class World {
    public:
        static World& getInstance();
        string getDimensionName() const;
        // Resolved from Properties::level_name at construction -- the
        // directory level.dat and region/ live in (see LevelDat, RegionFile).
        const string& getWorldDir() const;
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
        // data is never mutated after generation). Checks _chunkCache first --
        // a fully-lit chunk already contains everything terrain-only data
        // would (and is more up to date if it's been edited), so this never
        // duplicates a chunk's storage across both caches. Falls back to
        // _terrainCache / generation only for chunks not lit yet. Used both
        // by LightEngine's neighbor queries and internally by getChunkAsync,
        // so a chunk's terrain is only ever generated once no matter how many
        // nearby chunks need it purely for lighting occlusion. Safe from any thread.
        std::shared_ptr<Chunk> getOrGenerateTerrain(int chunkX, int chunkZ);
        // Synchronous read of an already-cached, fully-lit chunk -- nullptr if
        // not cached (not loaded/generated yet). Safe from any thread.
        std::shared_ptr<Chunk> getCachedChunk(int chunkX, int chunkZ);
        // Synchronously generates/loads + lights + caches (chunkX, chunkZ) if
        // not already cached -- a no-op otherwise. Unlike getChunkAsync, this
        // does the work on the calling thread rather than WorldWorkerPool, so
        // callers can guarantee a specific chunk is ready before proceeding.
        // Used at join time for a player's own standing chunk: teleporting a
        // player somewhere before their chunk is even queued for generation
        // (as an async dispatch would allow) lets the client simulate a
        // moment of free-fall through an unloaded column before real terrain
        // arrives, landing embedded a few blocks below the surface once it does.
        void ensureChunkLoaded(int chunkX, int chunkZ);
        // Applies one block edit at absolute world coordinates. Copy-then-replace
        // (see docs/general-documentation.md, "Terrain cache vs. lit cache"):
        // builds a modified copy of the cached chunk, relights just that copy,
        // then swaps it into _chunkCache -- the previous shared_ptr, if any
        // other thread is mid-read of it, is left untouched and internally
        // consistent. Returns false (no-op) if the chunk isn't cached.
        bool setBlock(int worldX, int worldY, int worldZ, Int32 blockStateId);
        // Drains and returns every chunk coordinate generated/edited since
        // the last call (AutosaveSystem / the shutdown save hook are the two
        // callers) -- locks just long enough to copy and clear the set, not
        // held during the disk I/O that follows, mirroring setBlock's own
        // "never hold the lock during the expensive part" caution.
        std::vector<std::pair<int,int>> takeDirtyChunksSnapshot();
        // Snapshot of current world-level state ready for LevelDat::save --
        // shared by AutosaveSystem and the shutdown save hook so neither
        // duplicates the Properties-field mapping.
        LevelData buildLevelData() const;
        // Reverse index of chunk -> number of players currently holding it
        // loaded, maintained by Connection::deliverGeneratedChunks (add),
        // UpdateLoadedChunks's unload loop (remove), and
        // ConnectionManager::processConnection's disconnect branch (remove
        // for everything a disconnecting player had). Drives ChunkUnloadSystem's
        // eviction sweep -- a coordinate becomes eligible once its count drops
        // to zero and stays that way past a grace period (see evictStaleChunks).
        void chunkViewerAdded(int chunkX, int chunkZ);
        void chunkViewerRemoved(int chunkX, int chunkZ);
        // Evicts (saving first if dirty) every chunk that's had zero viewers
        // for longer than gracePeriodSeconds. Called periodically by
        // ChunkUnloadSystem, never held during disk I/O.
        void evictStaleChunks(double gracePeriodSeconds);
        // Drops any _terrainCache entry that isn't currently viewed by any
        // player -- no grace period or save needed, these are never dirty.
        void evictStaleTerrainCache();
        // Ticks since world creation, persisted via buildLevelData/LevelDat --
        // no /time set command exists, so this doubles as both "World Age"
        // and "Time of day" for Update_Time_p (see DayNightSystem). Always
        // advances continuously; there's no gamerule system in this project
        // to hook a doDaylightCycle-style freeze into (deliberate, documented
        // gap, matching LevelData's own "game rules out of scope" stance).
        void advanceDayTime();
        Int64 getDayTime() const;
    private:
        World();
        // Inserts into _chunkCache and drops the now-redundant _terrainCache
        // entry for the same coordinate, if any -- called at every point a
        // chunk becomes fully lit, so a coordinate is never stored twice.
        // Marks the coordinate dirty (needs saving) unless it was just loaded
        // unmodified from disk (see getOrGenerateTerrain/_cleanChunks) --
        // an untouched imported chunk shouldn't get rewritten every autosave.
        void cacheAsLit(int chunkX, int chunkZ, std::shared_ptr<Chunk> chunk);
        // The actual eviction of one chunk, race-safe: re-checks the viewer
        // count in the same lock acquisition as the cache erase, so a viewer
        // that returned between the sweep deciding to evict and this running
        // aborts the eviction instead of silently discarding a stray in-flight
        // edit. Called by evictStaleChunks, never holds the lock during the
        // WorldPersistence save that follows.
        void evictChunk(int chunkX, int chunkZ);
        string _worldDir;
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
        // Both guarded by _chunkCacheMutex (reusing it avoids a second
        // lock-ordering concern). _cleanChunks holds coordinates that were
        // just loaded from disk unmodified -- consumed (erased) the first
        // time cacheAsLit runs for that coordinate, so a *later* edit via
        // setBlock still marks it dirty normally.
        std::set<std::pair<int,int>> _dirtyChunks;
        std::set<std::pair<int,int>> _cleanChunks;
        // Also guarded by _chunkCacheMutex. A coordinate is in exactly one of
        // these at a time (or neither, if never viewed -- see the spawn-chunk
        // note in World.cpp): _chunkViewerCounts holds only >0 entries;
        // _zeroViewerSince stamps the moment a coordinate's count reached zero,
        // so evictStaleChunks can tell how long it's been unviewed.
        std::map<std::pair<int,int>, int> _chunkViewerCounts;
        std::map<std::pair<int,int>, std::chrono::steady_clock::time_point> _zeroViewerSince;
        // Tick thread is the sole writer (DayNightSystem::onTick), connection
        // threads read it (join packet, periodic broadcast, buildLevelData) --
        // atomic rather than mutex-guarded since it's a single scalar with no
        // invariant to hold across other fields.
        std::atomic<Int64> _dayTime{0};
};