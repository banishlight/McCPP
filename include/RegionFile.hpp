#pragma once
#include <Standards.hpp>
#include <network/Nbt.hpp>
#include <array>
#include <fstream>
#include <mutex>
#include <optional>
#include <utility>

// One vanilla Anvil region file (region/r.{regionX}.{regionZ}.mca): a 32x32
// grid of chunks, each stored as a [4-byte length][1-byte compression
// scheme][compressed NBT] blob, sector-aligned (4096 bytes) after an 8KiB
// header (a 1024-entry location table, then a 1024-entry timestamp table).
// Chunk-agnostic -- treats each slot as an opaque NbtTag, with no knowledge of
// what a chunk's fields mean (see ChunkNbtCodec for that).
//
// Internally locked: multiple WorldWorkerPool threads may read/write
// different chunks in the same region concurrently.
class RegionFile {
    public:
        // Opens <worldDir>/region/r.{regionX}.{regionZ}.mca if it already
        // exists. Deliberately does NOT create it just from being
        // constructed/queried -- WorldPersistence opens a RegionFile for
        // every region a chunk lookup touches, including lighting's
        // neighbor-chunk queries near a region boundary, so eagerly creating
        // an empty file here would litter region/ with empty 8KiB files for
        // regions nothing was ever actually saved to. The file is created
        // lazily, only on the first real writeChunk().
        RegionFile(const string& worldDir, int regionX, int regionZ);

        // localX/localZ must be 0-31 (region-relative, i.e. chunkX & 31).
        // Returns std::nullopt if this chunk has never been saved, or if its
        // stored compression scheme isn't supported (logged, not fatal).
        std::optional<NbtTag> readChunk(int localX, int localZ);
        // Always writes using compression scheme 2 (zlib). Overwrites in
        // place if the new data fits the chunk's existing sector allocation,
        // otherwise appends at end-of-file (no compaction/free-list yet).
        void writeChunk(int localX, int localZ, const NbtTag& root);

        // Chunk coords -> region coords, floor-dividing by 32 (arithmetic
        // right shift floors correctly for negative values, unlike integer
        // division -- this is why Chunk.hpp needed its own floorDiv16 for /
        // but not for >>).
        static std::pair<int,int> regionCoordsFor(int chunkX, int chunkZ);

    private:
        struct LocationEntry {
            UInt32 sectorOffset = 0;
            Byte sectorCount = 0;
        };
        void loadHeader();
        void flushHeader();
        // Creates the (until-now nonexistent) file with a zeroed header and
        // opens it -- a no-op if already open. Called at the top of writeChunk.
        void ensureFileOpenForWrite();
        static int indexFor(int localX, int localZ) { return localX + localZ * 32; }

        string _path;
        std::fstream _file;
        std::array<LocationEntry, 1024> _locations{};
        std::array<UInt32, 1024> _timestamps{};
        std::mutex _mutex;
};
