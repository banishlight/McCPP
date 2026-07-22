#include <ChunkNbtCodec.hpp>
#include <BlockNames.hpp>
#include <BlockIds.hpp>
#include <FluidBlocks.hpp>
#include <VanillaVersion.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <algorithm>
#include <cmath>

namespace {
    constexpr int SECTION_BLOCKS = 4096;

    // bitsPerEntry = max(4, ceil(log2(paletteSize))), 0 for a single-entry
    // palette (no data array at all in that case -- vanilla's own optimization
    // for a uniform section, e.g. an all-air section).
    int paletteIndexBits(size_t paletteSize) {
        if (paletteSize <= 1) return 0;
        int bits = 0;
        while ((static_cast<size_t>(1) << bits) < paletteSize) bits++;
        return std::max(bits, 4);
    }

    // Unpacks `count` fixed-width entries from a bit-packed LongArray, using
    // the non-spanning scheme (in use since 1.16): entries never cross a long
    // boundary, so each long holds floor(64/bitsPerEntry) entries with any
    // leftover high bits unused. Needs real verification against a
    // known-good real chunk (see docs) -- this is exactly the kind of
    // version-specific wire detail this project has gotten wrong from memory
    // before, and a pure self-round-trip test can't reveal a bug that's
    // symmetric between this and packIndices below.
    std::vector<int> unpackIndices(const std::vector<Int64>& data, int bitsPerEntry, int count) {
        std::vector<int> indices(count, 0);
        if (bitsPerEntry <= 0) return indices;
        int entriesPerLong = 64 / bitsPerEntry;
        UInt64 mask = (bitsPerEntry >= 64) ? ~UInt64(0) : ((UInt64(1) << bitsPerEntry) - 1);
        for (int i = 0; i < count; i++) {
            int longIndex = i / entriesPerLong;
            if (static_cast<size_t>(longIndex) >= data.size()) break;
            int bitOffset = (i % entriesPerLong) * bitsPerEntry;
            UInt64 longValue = static_cast<UInt64>(data[longIndex]);
            indices[i] = static_cast<int>((longValue >> bitOffset) & mask);
        }
        return indices;
    }

    std::vector<Int64> packIndices(const std::vector<int>& indices, int bitsPerEntry) {
        if (bitsPerEntry <= 0) return {};
        int entriesPerLong = 64 / bitsPerEntry;
        int longCount = (static_cast<int>(indices.size()) + entriesPerLong - 1) / entriesPerLong;
        std::vector<Int64> data(longCount, 0);
        for (size_t i = 0; i < indices.size(); i++) {
            int longIndex = static_cast<int>(i) / entriesPerLong;
            int bitOffset = (static_cast<int>(i) % entriesPerLong) * bitsPerEntry;
            UInt64 value = static_cast<UInt64>(indices[i]);
            data[longIndex] |= static_cast<Int64>(value << bitOffset);
        }
        return data;
    }

    // Biome registry index lookups -- same "send order isn't fixed, look up
    // by id" convention already used for dimension_type/chat_type indices
    // elsewhere in this project.
    int biomeNameToIndex(const string& name) {
        const std::vector<RegistryEntry>& biomes = VanillaDataManager::getInstance().getEntries("worldgen/biome");
        for (size_t i = 0; i < biomes.size(); i++) {
            if (biomes[i].id == name) return static_cast<int>(i);
        }
        return 0;
    }

    string biomeIndexToName(int index) {
        const std::vector<RegistryEntry>& biomes = VanillaDataManager::getInstance().getEntries("worldgen/biome");
        if (index >= 0 && static_cast<size_t>(index) < biomes.size()) return biomes[index].id;
        return "minecraft:plains";
    }
}

namespace ChunkNbtCodec {

ChunkDecodeResult decodeChunk(const NbtTag& root, std::shared_ptr<Chunk>& out) {
    const NbtTag* dataVersionTag = root.get("DataVersion");
    if (!dataVersionTag) return ChunkDecodeResult::Malformed;
    if (dataVersionTag->asInt() < VanillaVersion::MIN_SUPPORTED_DATA_VERSION) {
        return ChunkDecodeResult::UnsupportedVersion;
    }

    const NbtTag* statusTag = root.get("Status");
    if (!statusTag) return ChunkDecodeResult::Malformed;
    if (statusTag->asString() != "minecraft:full") return ChunkDecodeResult::NotFullyGenerated;

    const NbtTag* xPosTag = root.get("xPos");
    const NbtTag* zPosTag = root.get("zPos");
    const NbtTag* sectionsTag = root.get("sections");
    if (!xPosTag || !zPosTag || !sectionsTag) return ChunkDecodeResult::Malformed;

    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(xPosTag->asInt(), zPosTag->asInt());
    int biomeIndexToSet = -1;

    for (const NbtTag& section : sectionsTag->asList()) {
        const NbtTag* yTag = section.get("Y");
        if (!yTag) continue;
        int sectionY = static_cast<int>(yTag->asLong());
        int worldYBase = sectionY * 16;
        if (worldYBase < Chunk::WORLD_MIN_Y || worldYBase >= Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT) {
            continue; // outside the Y range this project models
        }

        const NbtTag* blockStatesTag = section.get("block_states");
        if (blockStatesTag) {
            const NbtTag* paletteTag = blockStatesTag->get("palette");
            if (paletteTag) {
                const std::vector<NbtTag>& paletteList = paletteTag->asList();
                std::vector<Int32> resolvedIds;
                resolvedIds.reserve(paletteList.size());
                for (const NbtTag& entry : paletteList) {
                    const NbtTag* nameTag = entry.get("Name");
                    string name = nameTag ? nameTag->asString() : "minecraft:air";
                    const NbtTag* propsTag = entry.get("Properties");

                    // Water/lava are saved as a single block name ("minecraft:water"/
                    // "minecraft:lava") with a "level" property (0-15) -- not in
                    // BlockNames/BlockTable at all, since that table's one-name-per-
                    // state assumption doesn't fit a 16-state block family. Handled
                    // here directly rather than teaching BlockNames a second, wider
                    // property shape (int level vs bool snowy) it doesn't otherwise need.
                    Fluid::Type fluidType = (name == "minecraft:water") ? Fluid::Type::Water
                                          : (name == "minecraft:lava") ? Fluid::Type::Lava
                                          : Fluid::Type::None;
                    if (fluidType != Fluid::Type::None) {
                        int level = 0;
                        if (propsTag) {
                            const NbtTag* levelTag = propsTag->get("level");
                            if (levelTag) level = std::stoi(levelTag->asString());
                        }
                        resolvedIds.push_back(Fluid::sourceId(fluidType) + level);
                        continue;
                    }

                    bool snowy = false;
                    if (propsTag) {
                        const NbtTag* snowyTag = propsTag->get("snowy");
                        if (snowyTag && snowyTag->asString() == "true") snowy = true;
                    }
                    resolvedIds.push_back(BlockNames::blockNameToStateId(name, snowy));
                }

                if (resolvedIds.size() <= 1) {
                    // Single-block section -- data array is omitted entirely
                    // on disk, fill uniformly rather than treating this as an error.
                    Int32 fillId = resolvedIds.empty() ? AIR_BLOCK_STATE_ID : resolvedIds[0];
                    for (int y = 0; y < 16; y++) {
                        for (int z = 0; z < 16; z++) {
                            for (int x = 0; x < 16; x++) {
                                chunk->setBlock(x, worldYBase + y, z, fillId);
                            }
                        }
                    }
                } else {
                    const NbtTag* dataTag = blockStatesTag->get("data");
                    if (dataTag) {
                        int bitsPerEntry = paletteIndexBits(resolvedIds.size());
                        std::vector<int> indices = unpackIndices(dataTag->asLongArray(), bitsPerEntry, SECTION_BLOCKS);
                        for (int y = 0; y < 16; y++) {
                            for (int z = 0; z < 16; z++) {
                                for (int x = 0; x < 16; x++) {
                                    int i = (y << 8) | (z << 4) | x;
                                    int paletteIndex = indices[i];
                                    Int32 blockId = (paletteIndex >= 0 && static_cast<size_t>(paletteIndex) < resolvedIds.size())
                                        ? resolvedIds[paletteIndex] : AIR_BLOCK_STATE_ID;
                                    chunk->setBlock(x, worldYBase + y, z, blockId);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (biomeIndexToSet < 0) {
            const NbtTag* biomesTag = section.get("biomes");
            if (biomesTag) {
                const NbtTag* biomePaletteTag = biomesTag->get("palette");
                if (biomePaletteTag && !biomePaletteTag->asList().empty()) {
                    biomeIndexToSet = biomeNameToIndex(biomePaletteTag->asList()[0].asString());
                }
            }
        }
    }

    if (biomeIndexToSet >= 0) {
        chunk->setBiomeId(biomeIndexToSet);
    }

    out = chunk;
    return ChunkDecodeResult::Loaded;
}

NbtTag encodeChunk(const Chunk& chunk, int chunkX, int chunkZ) {
    NbtTag root = NbtTag::makeCompound();
    root.put("DataVersion", NbtTag::makeInt(VanillaVersion::CURRENT_DATA_VERSION));
    root.put("xPos", NbtTag::makeInt(chunkX));
    root.put("zPos", NbtTag::makeInt(chunkZ));
    root.put("yPos", NbtTag::makeInt(Chunk::WORLD_MIN_Y / 16));
    root.put("Status", NbtTag::makeString("minecraft:full"));
    root.put("isLightOn", NbtTag::makeByte(0));

    string biomeName = biomeIndexToName(chunk.getBiomeId());

    std::vector<NbtTag> sections;
    sections.reserve(Chunk::SECTION_COUNT);
    for (int s = 0; s < Chunk::SECTION_COUNT; s++) {
        int sectionY = (Chunk::WORLD_MIN_Y / 16) + s;
        int worldYBase = sectionY * 16;

        std::vector<Int32> paletteIds;
        std::vector<int> blockPaletteIndex(SECTION_BLOCKS, 0);
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    Int32 id = chunk.getBlock(x, worldYBase + y, z);
                    int i = (y << 8) | (z << 4) | x;
                    auto it = std::find(paletteIds.begin(), paletteIds.end(), id);
                    int idx;
                    if (it == paletteIds.end()) {
                        idx = static_cast<int>(paletteIds.size());
                        paletteIds.push_back(id);
                    } else {
                        idx = static_cast<int>(it - paletteIds.begin());
                    }
                    blockPaletteIndex[i] = idx;
                }
            }
        }

        NbtTag sectionTag = NbtTag::makeCompound();
        sectionTag.put("Y", NbtTag::makeByte(static_cast<Int8>(sectionY)));

        NbtTag blockStates = NbtTag::makeCompound();
        std::vector<NbtTag> paletteEntries;
        paletteEntries.reserve(paletteIds.size());
        for (Int32 id : paletteIds) {
            NbtTag entry = NbtTag::makeCompound();
            // See the matching decode-side comment: water/lava need a "level"
            // property BlockNames/BlockTable has no shape for, so they're
            // written directly here rather than through blockStateIdToName --
            // which, before this fix, had no entry for any fluid ID and
            // silently fell back to "minecraft:stone", the real cause of a
            // reported "water turns to stone after leaving and rejoining" bug
            // (every fluid block got saved as stone, then faithfully loaded
            // back as stone).
            Fluid::Type fluidType = Fluid::typeOf(id);
            if (fluidType != Fluid::Type::None) {
                int level = id - Fluid::sourceId(fluidType);
                entry.put("Name", NbtTag::makeString(fluidType == Fluid::Type::Water ? "minecraft:water" : "minecraft:lava"));
                NbtTag props = NbtTag::makeCompound();
                props.put("level", NbtTag::makeString(std::to_string(level)));
                entry.put("Properties", props);
            } else {
                entry.put("Name", NbtTag::makeString(BlockNames::blockStateIdToName(id)));
                if (id == GRASS_BLOCK_STATE_ID) {
                    NbtTag props = NbtTag::makeCompound();
                    props.put("snowy", NbtTag::makeString("false"));
                    entry.put("Properties", props);
                }
            }
            paletteEntries.push_back(entry);
        }
        blockStates.put("palette", NbtTag::makeList(NbtTagType::Compound, paletteEntries));
        if (paletteIds.size() > 1) {
            int bitsPerEntry = paletteIndexBits(paletteIds.size());
            blockStates.put("data", NbtTag::makeLongArray(packIndices(blockPaletteIndex, bitsPerEntry)));
        }
        sectionTag.put("block_states", blockStates);

        NbtTag biomes = NbtTag::makeCompound();
        biomes.put("palette", NbtTag::makeList(NbtTagType::String, { NbtTag::makeString(biomeName) }));
        sectionTag.put("biomes", biomes);

        sections.push_back(sectionTag);
    }
    root.put("sections", NbtTag::makeList(NbtTagType::Compound, sections));
    root.put("block_entities", NbtTag::makeList(NbtTagType::Compound, {}));

    return root;
}

}
