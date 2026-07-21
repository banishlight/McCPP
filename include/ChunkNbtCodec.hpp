#pragma once
#include <Standards.hpp>
#include <network/Nbt.hpp>
#include <Chunk.hpp>
#include <memory>

// Translates between a vanilla (1.18+) chunk NBT compound and this project's
// Chunk. Chunk-semantics layer -- RegionFile handles the on-disk container
// format and treats a chunk as an opaque NBT blob; this is what actually
// understands what's inside one. Light data is never read or written here at
// all: this project always recomputes lighting fresh via LightEngine
// regardless of a chunk's origin, and vanilla's own isLightOn=false already
// means "recompute me", so persisting light nibble arrays would be unused
// complexity. Entities/block_entities are read-but-discarded on decode and
// always written empty on encode -- this project has no mob or block-entity
// concept yet.
enum class ChunkDecodeResult {
    Loaded,            // sections list is a supported chunk
    NotFullyGenerated, // Status wasn't "minecraft:full" -- caller should fall back to generation
    UnsupportedVersion,// DataVersion below VanillaVersion::MIN_SUPPORTED_DATA_VERSION
    Malformed,         // missing/wrong-typed required fields
};

namespace ChunkNbtCodec {
    // On Loaded, *out is replaced with a freshly-built Chunk. Left untouched otherwise.
    ChunkDecodeResult decodeChunk(const NbtTag& root, std::shared_ptr<Chunk>& out);

    // Builds a chunk-root NBT compound (not yet gzipped/region-framed --
    // that's RegionFile::writeChunk's job) ready to hand to a region file.
    NbtTag encodeChunk(const Chunk& chunk, int chunkX, int chunkZ);
}
