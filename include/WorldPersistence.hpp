#pragma once
#include <Standards.hpp>
#include <Chunk.hpp>
#include <RegionFile.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

// Sits between World and RegionFile: owns a small cache of open region-file
// handles (keyed by region coordinate, lazily opened), and is what actually
// understands "a chunk" rather than "a region file" -- ChunkNbtCodec does the
// real translation, this just wires RegionFile + ChunkNbtCodec together and
// keeps region handles alive across many chunk loads/saves instead of
// reopening a file per chunk.
class WorldPersistence {
    public:
        static WorldPersistence& getInstance();

        // Must be called once, with World's resolved world directory, before
        // any load/save -- deliberately not read from World::getInstance()
        // itself here: World's own constructor calls getOrGenerateTerrain()
        // for its spawn-chunk priming, which reaches tryLoadChunk() below, and
        // World::getInstance() would then be called reentrantly on the same
        // thread while its Meyer's-singleton static local is still under
        // construction -- undefined behavior (libstdc++ throws on detecting
        // this). Storing the directory here instead avoids that call
        // entirely. World::World() calls this immediately after resolving
        // _worldDir, before doing anything else that could load a chunk.
        void initialize(const string& worldDir);

        // nullptr if this chunk was never saved, isn't fully generated, or is
        // from an unsupported (pre-1.18) world version -- callers fall back
        // to generation transparently in every case.
        std::shared_ptr<Chunk> tryLoadChunk(int chunkX, int chunkZ);
        void saveChunk(int chunkX, int chunkZ, const Chunk& chunk);

    private:
        WorldPersistence() = default;
        std::shared_ptr<RegionFile> getOrOpenRegion(int regionX, int regionZ);

        string _worldDir;
        std::mutex _mutex;
        std::map<std::pair<int,int>, std::shared_ptr<RegionFile>> _openRegions;
};
