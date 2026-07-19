#include <Standards.hpp>
#include <Properties.hpp>
#include <network/packets/Play.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Position.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <Player.hpp>
#include <World.hpp>
#include <BlockIds.hpp>
#include <ItemBlockMapping.hpp>
#include <EntityIdAllocator.hpp>
#include <network/Crypto.hpp>
#include <Console.hpp>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>
#include <functional>

Login_Play_p::Login_Play_p(int threshold, const Player& player) {
    _threshold = threshold;
    _entityId = player.getEntityId();
    _gamemode = player.getGamemode();
    _viewDistance = player.getViewDistance();
}

std::vector<Byte> Login_Play_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Login_Play_p::serialize(): Sending.");
    #endif
    World& world = World::getInstance();

    // Dimension Type is a VarInt index into the dimension_type registry, in the
    // exact order we already sent it via Registry_Data_p during Configuration.
    int dimensionTypeIndex = 0;
    const std::vector<RegistryEntry>& dimensionTypes = VanillaDataManager::getInstance().getEntries("dimension_type");
    for (size_t i = 0; i < dimensionTypes.size(); i++) {
        if (dimensionTypes[i].id == world.getDimensionName()) {
            dimensionTypeIndex = static_cast<int>(i);
            break;
        }
    }

    std::vector<Byte> packet_data;
    // Entity ID (Int)
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_entityId >> (i * 8)) & 0xFF));
    }
    packet_data.push_back(0x00); // Is hardcore

    // Dimension Names: only the one dimension we host
    std::vector<Byte> dimensionCount = varIntSerialize(1);
    packet_data.insert(packet_data.end(), dimensionCount.begin(), dimensionCount.end());
    std::vector<Byte> dimensionNameBytes = serializeString(world.getDimensionName());
    packet_data.insert(packet_data.end(), dimensionNameBytes.begin(), dimensionNameBytes.end());

    std::vector<Byte> maxPlayers = varIntSerialize(Properties::getProperties().max_players);
    packet_data.insert(packet_data.end(), maxPlayers.begin(), maxPlayers.end());
    std::vector<Byte> viewDistance = varIntSerialize(_viewDistance);
    packet_data.insert(packet_data.end(), viewDistance.begin(), viewDistance.end());
    std::vector<Byte> simulationDistance = varIntSerialize(_viewDistance);
    packet_data.insert(packet_data.end(), simulationDistance.begin(), simulationDistance.end());

    packet_data.push_back(0x00); // Reduced Debug Info
    packet_data.push_back(0x01); // Enable respawn screen
    packet_data.push_back(0x00); // Do limited crafting

    std::vector<Byte> dimTypeIdx = varIntSerialize(dimensionTypeIndex);
    packet_data.insert(packet_data.end(), dimTypeIdx.begin(), dimTypeIdx.end());
    std::vector<Byte> dimensionName2 = serializeString(world.getDimensionName());
    packet_data.insert(packet_data.end(), dimensionName2.begin(), dimensionName2.end());

    // Hashed seed (Long)
    Int64 hashedSeed = world.getHashedSeed();
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((hashedSeed >> (i * 8)) & 0xFF));
    }

    packet_data.push_back(static_cast<Byte>(_gamemode)); // Game mode
    packet_data.push_back(0xFF); // Previous Game mode: -1 (undefined)
    packet_data.push_back(0x00); // Is Debug
    packet_data.push_back(world.isFlat() ? 0x01 : 0x00); // Is Flat
    packet_data.push_back(0x00); // Has death location

    std::vector<Byte> portalCooldown = varIntSerialize(0);
    packet_data.insert(packet_data.end(), portalCooldown.begin(), portalCooldown.end());
    packet_data.push_back(0x00); // Enforces Secure Chat

    return assemblePacket(getID(), _threshold, packet_data);
}

Synchronize_Player_Position_p::Synchronize_Player_Position_p(int threshold, double x, double y, double z, float yaw, float pitch, int teleportId) {
    _threshold = threshold;
    _x = x;
    _y = y;
    _z = z;
    _yaw = yaw;
    _pitch = pitch;
    _teleportId = teleportId;
}

std::vector<Byte> Synchronize_Player_Position_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Synchronize_Player_Position_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;

    Int64 xBits, yBits, zBits;
    static_assert(sizeof(double) == sizeof(Int64), "double must be 64-bit for this cast");
    std::memcpy(&xBits, &_x, sizeof(Int64));
    std::memcpy(&yBits, &_y, sizeof(Int64));
    std::memcpy(&zBits, &_z, sizeof(Int64));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((xBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((yBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((zBits >> (i * 8)) & 0xFF));

    Int32 yawBits, pitchBits;
    std::memcpy(&yawBits, &_yaw, sizeof(Int32));
    std::memcpy(&pitchBits, &_pitch, sizeof(Int32));
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((yawBits >> (i * 8)) & 0xFF));
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((pitchBits >> (i * 8)) & 0xFF));

    packet_data.push_back(0x00); // Flags: all absolute

    std::vector<Byte> teleportId = varIntSerialize(_teleportId);
    packet_data.insert(packet_data.end(), teleportId.begin(), teleportId.end());

    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Default_Spawn_Position_p::Set_Default_Spawn_Position_p(int threshold) {
    _threshold = threshold;
}

std::vector<Byte> Set_Default_Spawn_Position_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Default_Spawn_Position_p::serialize(): Sending.");
    #endif
    World& world = World::getInstance();
    std::vector<Byte> packet_data;

    Int64 location = EncodePosition(static_cast<Int32>(world.getSpawnX()), static_cast<Int32>(world.getSpawnY()), static_cast<Int32>(world.getSpawnZ()));
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((location >> (i * 8)) & 0xFF));
    }

    float angle = world.getSpawnYaw();
    Int32 angleBits;
    std::memcpy(&angleBits, &angle, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((angleBits >> (i * 8)) & 0xFF));
    }

    return assemblePacket(getID(), _threshold, packet_data);
}

Clientbound_Keep_Alive_play_p::Clientbound_Keep_Alive_play_p(int threshold, Int64 keepAliveId) {
    _threshold = threshold;
    _keepAliveId = keepAliveId;
}

std::vector<Byte> Clientbound_Keep_Alive_play_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Clientbound_Keep_Alive_play_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_keepAliveId >> (i * 8)) & 0xFF));
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

Game_Event_p::Game_Event_p(int threshold, Byte event, float value) {
    _threshold = threshold;
    _event = event;
    _value = value;
}

std::vector<Byte> Game_Event_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Game_Event_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    packet_data.push_back(_event);
    Int32 valueBits;
    std::memcpy(&valueBits, &_value, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((valueBits >> (i * 8)) & 0xFF));
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Center_Chunk_p::Set_Center_Chunk_p(int threshold, int chunkX, int chunkZ) {
    _threshold = threshold;
    _chunkX = chunkX;
    _chunkZ = chunkZ;
}

std::vector<Byte> Set_Center_Chunk_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Center_Chunk_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_chunkX);
    std::vector<Byte> chunkZBytes = varIntSerialize(_chunkZ);
    packet_data.insert(packet_data.end(), chunkZBytes.begin(), chunkZBytes.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

namespace {
    // Encodes one section's Paletted Container (indirect palette, bit-packed
    // into longs) -- see docs/network-protocol.md for the wire format.
    void encodeSection(std::vector<Byte>& out, const std::vector<Int32>& blockIds) {
        std::vector<Int32> palette;
        std::vector<int> indices(blockIds.size());
        for (size_t i = 0; i < blockIds.size(); i++) {
            Int32 id = blockIds[i];
            int idx = -1;
            for (size_t p = 0; p < palette.size(); p++) {
                if (palette[p] == id) { idx = static_cast<int>(p); break; }
            }
            if (idx < 0) {
                idx = static_cast<int>(palette.size());
                palette.push_back(id);
            }
            indices[i] = idx;
        }

        // Single-valued (Bits Per Entry = 0) still needs a Data Array Length
        // VarInt (always 0 here) -- present but zero-length, not omitted. See
        // docs/general-documentation.md, "Wire format gotchas found the hard way".
        if (palette.size() == 1) {
            out.push_back(0x00);
            std::vector<Byte> valueBytes = varIntSerialize(palette[0]);
            out.insert(out.end(), valueBytes.begin(), valueBytes.end());
            std::vector<Byte> zeroLen = varIntSerialize(0);
            out.insert(out.end(), zeroLen.begin(), zeroLen.end());
            return;
        }

        int bits = 4;
        while ((1u << bits) < palette.size()) bits++;
        // Block variety never approaches the 8-bit indirect ceiling, so the
        // direct/global-palette fallback isn't needed and isn't built.

        out.push_back(static_cast<Byte>(bits));
        std::vector<Byte> paletteCount = varIntSerialize(static_cast<int>(palette.size()));
        out.insert(out.end(), paletteCount.begin(), paletteCount.end());
        for (Int32 id : palette) {
            std::vector<Byte> idBytes = varIntSerialize(id);
            out.insert(out.end(), idBytes.begin(), idBytes.end());
        }

        int entriesPerLong = 64 / bits;
        int longCount = (static_cast<int>(indices.size()) + entriesPerLong - 1) / entriesPerLong;
        std::vector<Byte> lengthBytes = varIntSerialize(longCount);
        out.insert(out.end(), lengthBytes.begin(), lengthBytes.end());

        UInt64 mask = (1ULL << bits) - 1;
        for (int l = 0; l < longCount; l++) {
            UInt64 word = 0;
            for (int e = 0; e < entriesPerLong; e++) {
                size_t entryIndex = static_cast<size_t>(l) * entriesPerLong + e;
                if (entryIndex >= indices.size()) break;
                word |= (static_cast<UInt64>(indices[entryIndex]) & mask) << (e * bits);
            }
            for (int i = 7; i >= 0; i--) {
                out.push_back(static_cast<Byte>((word >> (i * 8)) & 0xFF));
            }
        }
    }

    // BitSet: VarInt count of longs, then that many big-endian longs, LSB of
    // the first long = bit 0 (docs/network-protocol.md).
    std::vector<Byte> makeBitSet(const std::vector<int>& setBits, int totalBits) {
        int longCount = (totalBits + 63) / 64;
        std::vector<UInt64> longs(longCount, 0);
        for (int bit : setBits) {
            longs[bit / 64] |= (1ULL << (bit % 64));
        }
        std::vector<Byte> result = varIntSerialize(longCount);
        for (UInt64 l : longs) {
            for (int i = 7; i >= 0; i--) {
                result.push_back(static_cast<Byte>((l >> (i * 8)) & 0xFF));
            }
        }
        return result;
    }

    // Packs 4096 light values (0-15) into a 2048-byte nibble array, indexed
    // ((y<<8)|(z<<4)|x)/2 -- low nibble on even index, high nibble on odd
    // (docs/network-protocol.md).
    std::vector<Byte> packLightSection(const std::function<int(int, int, int)>& getLevel) {
        std::vector<Byte> out(2048, 0);
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    int i = (y << 8) | (z << 4) | x;
                    int level = getLevel(x, y, z) & 0x0F;
                    int byteIndex = i / 2;
                    if (i % 2 == 0) {
                        out[byteIndex] = static_cast<Byte>((out[byteIndex] & 0xF0) | level);
                    } else {
                        out[byteIndex] = static_cast<Byte>((out[byteIndex] & 0x0F) | (level << 4));
                    }
                }
            }
        }
        return out;
    }

    bool isAllZero(const std::vector<Byte>& packed) {
        for (Byte b : packed) {
            if (b != 0) return false;
        }
        return true;
    }

    // Slot (1.20.5+ Data Components format, cross-checked against Pumpkin's
    // ItemStackSerializer since docs/network-protocol.md only links out to an
    // external page for this type): empty is a single VarInt(0). Non-empty is
    // VarInt(count), VarInt(item ID), then VarInt(components-to-add count)
    // and VarInt(components-to-remove count) -- both 0 here, since nothing
    // this server places needs a non-default component (custom name, damage,
    // etc.), so no component entries ever follow.
    std::vector<Byte> packSlot(Int32 itemId, Int32 count) {
        if (itemId < 0 || count <= 0) {
            return varIntSerialize(0);
        }
        std::vector<Byte> out = varIntSerialize(count);
        std::vector<Byte> idBytes = varIntSerialize(itemId);
        out.insert(out.end(), idBytes.begin(), idBytes.end());
        std::vector<Byte> addCount = varIntSerialize(0);
        out.insert(out.end(), addCount.begin(), addCount.end());
        std::vector<Byte> removeCount = varIntSerialize(0);
        out.insert(out.end(), removeCount.begin(), removeCount.end());
        return out;
    }

    // MOTION_BLOCKING heightmap: for each of the 256 columns, the highest
    // non-air block's Y, stored as (y - WORLD_MIN_Y + 1) so 0 means "no
    // blocking block in this column" -- 9 bits/entry, 7 entries/long (never
    // straddling), index = z*16+x. Matches vanilla's own packing scheme.
    std::vector<Int64> computeMotionBlockingHeightmap(const Chunk& chunk) {
        constexpr int BITS = 9;
        constexpr int ENTRIES_PER_LONG = 64 / BITS;
        constexpr int LONG_COUNT = (256 + ENTRIES_PER_LONG - 1) / ENTRIES_PER_LONG;
        std::vector<Int64> data(LONG_COUNT, 0);
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                int height = Chunk::WORLD_MIN_Y - 1;
                for (int worldY = Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1; worldY >= Chunk::WORLD_MIN_Y; worldY--) {
                    if (chunk.getBlock(x, worldY, z) != AIR_BLOCK_STATE_ID) {
                        height = worldY;
                        break;
                    }
                }
                UInt64 value = static_cast<UInt64>(height - Chunk::WORLD_MIN_Y + 1) & 0x1FF;
                int columnIdx = z * 16 + x;
                int longIdx = columnIdx / ENTRIES_PER_LONG;
                int shift = (columnIdx % ENTRIES_PER_LONG) * BITS;
                data[longIdx] |= static_cast<Int64>(value << shift);
            }
        }
        return data;
    }
}

Chunk_Data_p::Chunk_Data_p(int threshold, std::shared_ptr<Chunk> chunk) {
    _threshold = threshold;
    _chunk = chunk;
}

std::vector<Byte> Chunk_Data_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Chunk_Data_p::serialize(): Sending.");
    #endif
    int chunkX = _chunk->getChunkX();
    int chunkZ = _chunk->getChunkZ();

    std::vector<Byte> packet_data;
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((chunkX >> (i * 8)) & 0xFF));
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((chunkZ >> (i * 8)) & 0xFF));

    NbtTag heightmaps = NbtTag::makeCompound();
    heightmaps.put("MOTION_BLOCKING", NbtTag::makeLongArray(computeMotionBlockingHeightmap(*_chunk)));
    std::vector<Byte> heightmapsBytes = heightmaps.serializeNetwork();
    packet_data.insert(packet_data.end(), heightmapsBytes.begin(), heightmapsBytes.end());

    const std::vector<Byte> ZERO_DATA_ARRAY_LENGTH = varIntSerialize(0);

    std::vector<Byte> sectionData;
    for (int s = 0; s < Chunk::SECTION_COUNT; s++) {
        int sectionWorldYBase = Chunk::WORLD_MIN_Y + s * 16;
        // Wire iteration order for a section's Paletted Container: Y outer,
        // then Z, then X (index = y*256 + z*16 + x within the section).
        std::vector<Int32> blockIds;
        blockIds.reserve(4096);
        Int16 nonAirCount = 0;
        for (int y = 0; y < 16; y++) {
            int worldY = sectionWorldYBase + y;
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    Int32 id = _chunk->getBlock(x, worldY, z);
                    blockIds.push_back(id);
                    if (id != AIR_BLOCK_STATE_ID) nonAirCount++;
                }
            }
        }
        // One combined non-empty-block count (blocks + fluids), not two
        // separate counts -- see docs/general-documentation.md, "Wire format
        // gotchas found the hard way".
        sectionData.push_back(static_cast<Byte>((nonAirCount >> 8) & 0xFF));
        sectionData.push_back(static_cast<Byte>(nonAirCount & 0xFF));
        encodeSection(sectionData, blockIds); // Block states container

        // Biomes: one global biome for the whole chunk, so this always stays
        // on the single-valued path regardless of how varied the terrain is.
        sectionData.push_back(0x00);
        std::vector<Byte> biomeId = varIntSerialize(_chunk->getBiomeId());
        sectionData.insert(sectionData.end(), biomeId.begin(), biomeId.end());
        sectionData.insert(sectionData.end(), ZERO_DATA_ARRAY_LENGTH.begin(), ZERO_DATA_ARRAY_LENGTH.end());
    }
    std::vector<Byte> sizeBytes = varIntSerialize(static_cast<int>(sectionData.size()));
    packet_data.insert(packet_data.end(), sizeBytes.begin(), sizeBytes.end());
    packet_data.insert(packet_data.end(), sectionData.begin(), sectionData.end());

    std::vector<Byte> blockEntityCount = varIntSerialize(0);
    packet_data.insert(packet_data.end(), blockEntityCount.begin(), blockEntityCount.end());

    // 26 light sections: bit 0 = below-world sentinel, bits 1..SECTION_COUNT
    // = real sections in order, last bit = above-world sentinel (see
    // docs/network-protocol.md for the BitSet/empty-mask wire format).
    const int LIGHT_SECTION_COUNT = Chunk::SECTION_COUNT + 2;
    const std::vector<Byte> allZeroLight(2048, 0);
    const std::vector<Byte> allFullSky(2048, 0xFF); // every nibble = 15

    std::vector<int> skyDataBits, skyEmptyBits, blockDataBits, blockEmptyBits;
    std::vector<std::vector<Byte>> skyArrays, blockArrays;

    for (int lightIndex = 0; lightIndex < LIGHT_SECTION_COUNT; lightIndex++) {
        std::vector<Byte> skyPacked;
        std::vector<Byte> blockPacked;
        if (lightIndex == 0) {
            skyPacked = allZeroLight; // below-world: nothing is ever generated there
            blockPacked = allZeroLight;
        } else if (lightIndex == LIGHT_SECTION_COUNT - 1) {
            skyPacked = allFullSky; // above-world: always open sky
            blockPacked = allZeroLight;
        } else {
            int sectionWorldYBase = Chunk::WORLD_MIN_Y + (lightIndex - 1) * 16;
            skyPacked = packLightSection([&](int x, int y, int z) {
                return _chunk->getSkyLight(x, sectionWorldYBase + y, z);
            });
            blockPacked = packLightSection([&](int x, int y, int z) {
                return _chunk->getBlockLight(x, sectionWorldYBase + y, z);
            });
        }

        if (isAllZero(skyPacked)) {
            skyEmptyBits.push_back(lightIndex);
        } else {
            skyDataBits.push_back(lightIndex);
            skyArrays.push_back(skyPacked);
        }
        if (isAllZero(blockPacked)) {
            blockEmptyBits.push_back(lightIndex);
        } else {
            blockDataBits.push_back(lightIndex);
            blockArrays.push_back(blockPacked);
        }
    }

    std::vector<Byte> skyMask = makeBitSet(skyDataBits, LIGHT_SECTION_COUNT);
    std::vector<Byte> blockMask = makeBitSet(blockDataBits, LIGHT_SECTION_COUNT);
    std::vector<Byte> emptySkyMask = makeBitSet(skyEmptyBits, LIGHT_SECTION_COUNT);
    std::vector<Byte> emptyBlockMask = makeBitSet(blockEmptyBits, LIGHT_SECTION_COUNT);
    packet_data.insert(packet_data.end(), skyMask.begin(), skyMask.end());
    packet_data.insert(packet_data.end(), blockMask.begin(), blockMask.end());
    packet_data.insert(packet_data.end(), emptySkyMask.begin(), emptySkyMask.end());
    packet_data.insert(packet_data.end(), emptyBlockMask.begin(), emptyBlockMask.end());

    std::vector<Byte> skyArrayCount = varIntSerialize(static_cast<int>(skyArrays.size()));
    packet_data.insert(packet_data.end(), skyArrayCount.begin(), skyArrayCount.end());
    for (const auto& arr : skyArrays) {
        std::vector<Byte> lenBytes = varIntSerialize(static_cast<int>(arr.size()));
        packet_data.insert(packet_data.end(), lenBytes.begin(), lenBytes.end());
        packet_data.insert(packet_data.end(), arr.begin(), arr.end());
    }

    std::vector<Byte> blockArrayCount = varIntSerialize(static_cast<int>(blockArrays.size()));
    packet_data.insert(packet_data.end(), blockArrayCount.begin(), blockArrayCount.end());
    for (const auto& arr : blockArrays) {
        std::vector<Byte> lenBytes = varIntSerialize(static_cast<int>(arr.size()));
        packet_data.insert(packet_data.end(), lenBytes.begin(), lenBytes.end());
        packet_data.insert(packet_data.end(), arr.begin(), arr.end());
    }

    return assemblePacket(getID(), _threshold, packet_data);
}

Unload_Chunk_p::Unload_Chunk_p(int threshold, int chunkX, int chunkZ) {
    _threshold = threshold;
    _chunkX = chunkX;
    _chunkZ = chunkZ;
}

std::vector<Byte> Unload_Chunk_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Unload_Chunk_p::serialize(): Sending.");
    #endif
    // Field order is Z then X -- the client reads this as one big-endian Long
    // with Z in the upper 32 bits, per docs/network-protocol.md.
    std::vector<Byte> packet_data;
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((_chunkZ >> (i * 8)) & 0xFF));
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((_chunkX >> (i * 8)) & 0xFF));
    return assemblePacket(getID(), _threshold, packet_data);
}

Block_Update_p::Block_Update_p(int threshold, Int32 x, Int32 y, Int32 z, Int32 blockStateId) {
    _threshold = threshold;
    _x = x;
    _y = y;
    _z = z;
    _blockStateId = blockStateId;
}

std::vector<Byte> Block_Update_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Block_Update_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    Int64 location = EncodePosition(_x, _y, _z);
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((location >> (i * 8)) & 0xFF));
    std::vector<Byte> blockIdBytes = varIntSerialize(_blockStateId);
    packet_data.insert(packet_data.end(), blockIdBytes.begin(), blockIdBytes.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

Acknowledge_Block_Change_p::Acknowledge_Block_Change_p(int threshold, int sequence) {
    _threshold = threshold;
    _sequence = sequence;
}

std::vector<Byte> Acknowledge_Block_Change_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Acknowledge_Block_Change_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_sequence);
    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Container_Content_p::Set_Container_Content_p(int threshold, const std::array<HotbarSlot, Player::HOTBAR_SIZE>& hotbar) {
    _threshold = threshold;
    _hotbar = hotbar;
}

std::vector<Byte> Set_Container_Content_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Container_Content_p::serialize(): Sending.");
    #endif
    // Player inventory (Window ID 0) container-slot layout: 0 = crafting
    // result, 1-4 = crafting grid, 5-8 = armor, 9-35 = main inventory,
    // 36-44 = hotbar, 45 = offhand (cross-checked against Pumpkin's
    // player_screen_handler.rs). The client maps array index directly to
    // this absolute slot index, so all 46 must be sent even though only the
    // hotbar range is ever populated -- sending fewer would misplace the
    // hotbar's contents into the crafting/armor slots instead.
    const int TOTAL_SLOTS = 46;
    const int HOTBAR_START = 36;

    std::vector<Byte> packet_data;
    packet_data.push_back(0x00); // Window ID: 0 = player inventory
    std::vector<Byte> stateId = varIntSerialize(0); // no Click Container handling, so state tracking is moot
    packet_data.insert(packet_data.end(), stateId.begin(), stateId.end());
    std::vector<Byte> count = varIntSerialize(TOTAL_SLOTS);
    packet_data.insert(packet_data.end(), count.begin(), count.end());
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        std::vector<Byte> slotBytes;
        if (i >= HOTBAR_START && i < HOTBAR_START + Player::HOTBAR_SIZE) {
            const HotbarSlot& slot = _hotbar[i - HOTBAR_START];
            slotBytes = packSlot(slot.itemId, slot.count);
        } else {
            slotBytes = packSlot(-1, 0);
        }
        packet_data.insert(packet_data.end(), slotBytes.begin(), slotBytes.end());
    }
    std::vector<Byte> carried = packSlot(-1, 0); // nothing dragged with the mouse
    packet_data.insert(packet_data.end(), carried.begin(), carried.end());

    return assemblePacket(getID(), _threshold, packet_data);
}

Spawn_Entity_p::Spawn_Entity_p(int threshold, int entityId, const std::vector<long>& uuid, int entityTypeId, double x, double y, double z) {
    _threshold = threshold;
    _entityId = entityId;
    _uuid = uuid;
    _entityTypeId = entityTypeId;
    _x = x;
    _y = y;
    _z = z;
}

std::vector<Byte> Spawn_Entity_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Spawn_Entity_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    std::vector<Byte> uuidBytes = serializeUUID(_uuid);
    packet_data.insert(packet_data.end(), uuidBytes.begin(), uuidBytes.end());
    std::vector<Byte> typeBytes = varIntSerialize(_entityTypeId);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());

    Int64 xBits, yBits, zBits;
    std::memcpy(&xBits, &_x, sizeof(Int64));
    std::memcpy(&yBits, &_y, sizeof(Int64));
    std::memcpy(&zBits, &_z, sizeof(Int64));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((xBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((yBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((zBits >> (i * 8)) & 0xFF));

    packet_data.push_back(0x00); // Pitch (Angle): a static drop has no meaningful rotation
    packet_data.push_back(0x00); // Yaw
    packet_data.push_back(0x00); // Head Yaw

    std::vector<Byte> dataBytes = varIntSerialize(0); // Object Data: unused for item entities
    packet_data.insert(packet_data.end(), dataBytes.begin(), dataBytes.end());

    for (int i = 0; i < 3; i++) { // Velocity X/Y/Z: 0 -- no toss/physics on this drop
        packet_data.push_back(0x00);
        packet_data.push_back(0x00);
    }

    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Entity_Metadata_p::Set_Entity_Metadata_p(int threshold, int entityId, Int32 itemId, Int32 count) {
    _threshold = threshold;
    _entityId = entityId;
    _itemId = itemId;
    _count = count;
}

std::vector<Byte> Set_Entity_Metadata_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Entity_Metadata_p::serialize(): Sending.");
    #endif
    // ItemEntity's "Item" field sits at metadata index 8, right after
    // Entity's own 8 base tracked fields (index 0-7); item_stack is metadata
    // type 7. Both sourced from Pumpkin's version-pinned 1.21 tracked_data/
    // meta_data_type tables, not guessed -- see docs/general-documentation.md.
    const Byte ITEM_ENTITY_METADATA_INDEX = 8;
    const int ITEM_STACK_METADATA_TYPE = 7;

    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(ITEM_ENTITY_METADATA_INDEX);
    std::vector<Byte> typeBytes = varIntSerialize(ITEM_STACK_METADATA_TYPE);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());
    std::vector<Byte> slotBytes = packSlot(_itemId, _count);
    packet_data.insert(packet_data.end(), slotBytes.begin(), slotBytes.end());
    packet_data.push_back(0xFF); // terminator: no more metadata entries

    return assemblePacket(getID(), _threshold, packet_data);
}

void BroadcastToChunkViewers(int chunkX, int chunkZ, const std::function<std::shared_ptr<Outgoing_Packet>(int threshold)>& makePacket) {
    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& conn : connections) {
        if (!conn) continue;
        if (conn->getState() != ConnectionState::Play) continue;
        if (!conn->getPlayer().hasChunkLoaded(chunkX, chunkZ)) continue;
        conn->addPacket(makePacket(conn->getCompressionThreshold()));
    }
}

void UpdateLoadedChunks(PacketContext& cont, int threshold, Player& player, int newCenterX, int newCenterZ) {
    // Cheap bail for the common case: a movement packet that hasn't crossed a
    // chunk boundary. getLoadedChunks().empty() lets the very first call
    // through even though the default center already matches (0,0).
    if (newCenterX == player.getCenterChunkX() && newCenterZ == player.getCenterChunkZ() && !player.getLoadedChunks().empty()) {
        return;
    }

    World& world = World::getInstance();
    int viewDistance = player.getViewDistance();

    cont.connection.addPacket(std::make_shared<Set_Center_Chunk_p>(threshold, newCenterX, newCenterZ));

    std::set<std::pair<int, int>> newChunks;
    for (int x = newCenterX - viewDistance; x <= newCenterX + viewDistance; x++) {
        for (int z = newCenterZ - viewDistance; z <= newCenterZ + viewDistance; z++) {
            newChunks.insert({x, z});
        }
    }

    // Dispatch nearest-to-center first -- see docs/general-documentation.md,
    // "Chunk dispatch/delivery ordering".
    std::vector<std::pair<int, int>> dispatchOrder(newChunks.begin(), newChunks.end());
    std::sort(dispatchOrder.begin(), dispatchOrder.end(), [newCenterX, newCenterZ](const auto& a, const auto& b) {
        int da = std::abs(a.first - newCenterX) + std::abs(a.second - newCenterZ);
        int db = std::abs(b.first - newCenterX) + std::abs(b.second - newCenterZ);
        return da < db;
    });

    // Generation happens on WorldWorkerPool and is delivered back through
    // Connection::addGeneratedChunk -- player.markChunkLoaded() happens later,
    // in Connection::deliverGeneratedChunks() on this connection's own thread,
    // once generation actually completes (not here, and not on a pool thread).
    std::shared_ptr<Connection> connPtr = cont.connection.shared_from_this();
    for (const auto& [x, z] : dispatchOrder) {
        if (!player.hasChunkLoaded(x, z)) {
            world.getChunkAsync(x, z, [connPtr](std::shared_ptr<Chunk> chunk) {
                connPtr->addGeneratedChunk(chunk);
            });
        }
    }

    std::set<std::pair<int, int>> previouslyLoaded = player.getLoadedChunks();
    for (const auto& [x, z] : previouslyLoaded) {
        if (!newChunks.count({x, z})) {
            cont.connection.addPacket(std::make_shared<Unload_Chunk_p>(threshold, x, z));
            player.markChunkUnloaded(x, z);
        }
    }

    player.setCenterChunk(newCenterX, newCenterZ);
}

void Confirm_Teleportation_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Confirm_Teleportation_p::deserialize(): Received.");
    #endif
    // TODO: validate against the teleport ID we last sent once multiple
    // in-flight teleports need disambiguating; only one is ever sent today.
}

void Set_Player_Position_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Player_Position_p::deserialize(): Received.");
    #endif
    double x = deserializeDouble(in_buff);
    double y = deserializeDouble(in_buff);
    double z = deserializeDouble(in_buff);
    // On Ground (Boolean) follows; unused, no movement validation yet.

    Player& player = cont.connection.getPlayer();
    player.setPosition(x, y, z);

    int threshold = cont.connection.getCompressionThreshold();
    int newCenterX = static_cast<int>(std::floor(x / 16.0));
    int newCenterZ = static_cast<int>(std::floor(z / 16.0));
    UpdateLoadedChunks(cont, threshold, player, newCenterX, newCenterZ);
}

void Set_Player_Position_and_Rotation_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Player_Position_and_Rotation_p::deserialize(): Received.");
    #endif
    double x = deserializeDouble(in_buff);
    double y = deserializeDouble(in_buff);
    double z = deserializeDouble(in_buff);
    float yaw = deserializeFloat(in_buff);
    float pitch = deserializeFloat(in_buff);
    // On Ground (Boolean) follows; unused, no movement validation yet.

    Player& player = cont.connection.getPlayer();
    player.setPosition(x, y, z);
    player.setRotation(yaw, pitch);

    int threshold = cont.connection.getCompressionThreshold();
    int newCenterX = static_cast<int>(std::floor(x / 16.0));
    int newCenterZ = static_cast<int>(std::floor(z / 16.0));
    UpdateLoadedChunks(cont, threshold, player, newCenterX, newCenterZ);
}

void Serverbound_Keep_Alive_play_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Serverbound_Keep_Alive_play_p::deserialize(): Received.");
    #endif
    // TODO: validate against the ID we last sent once Connection proactively
    // sends Keep Alives on a timer; nothing does yet, so there's nothing to check.
}

void Player_Action_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Player_Action_p::deserialize(): Received.");
    #endif
    const int CREATIVE_GAMEMODE = 1;

    int status = deserializeVarInt(in_buff);
    DecodedPosition loc = DecodePosition(deserializeLong(in_buff));
    deserializeByte(in_buff); // Face: unused for breaking, the clicked block itself is removed.
    int sequence = deserializeVarInt(in_buff);

    Player& player = cont.connection.getPlayer();
    // No server-side mining-time/hardness validation (see docs/general-documentation.md,
    // "Accepted gaps") -- trust status 2 (Finished digging) in any gamemode,
    // or status 0 (Started digging) in Creative, where the client never sends
    // a Finished digging follow-up.
    bool shouldBreak = (status == 2) || (status == 0 && player.getGamemode() == CREATIVE_GAMEMODE);
    if (shouldBreak) {
        // minecraft:item entity-type registry ID, sourced from the vanilla
        // data generator report, not guessed.
        const int ITEM_ENTITY_TYPE_ID = 58;

        World& world = World::getInstance();
        int chunkX = floorDiv16(loc.x);
        int chunkZ = floorDiv16(loc.z);

        // Read the block being broken before it's gone, so its drop can be
        // resolved -- World::setBlock alone doesn't report what was there.
        Int32 previousBlock = AIR_BLOCK_STATE_ID;
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (chunk) {
            previousBlock = chunk->getBlock(loc.x - chunkX * 16, loc.y, loc.z - chunkZ * 16);
        }

        world.setBlock(loc.x, loc.y, loc.z, AIR_BLOCK_STATE_ID);
        BroadcastToChunkViewers(chunkX, chunkZ, [loc](int threshold) {
            return std::make_shared<Block_Update_p>(threshold, loc.x, loc.y, loc.z, AIR_BLOCK_STATE_ID);
        });
        int threshold = cont.connection.getCompressionThreshold();
        cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));

        // No pickup/despawn logic -- the drop is purely visual, not tracked
        // once spawned (see docs/general-documentation.md, "Accepted gaps").
        Int32 dropItemId = blockStateIdToItemId(previousBlock);
        if (dropItemId >= 0) {
            int entityId = EntityIdAllocator::next();
            std::vector<long> uuid = generateRandomUUID();
            double dropX = loc.x + 0.5, dropY = loc.y + 0.5, dropZ = loc.z + 0.5;
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, dropX, dropY, dropZ, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
                return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, dropX, dropY, dropZ);
            });
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, dropItemId](int broadcastThreshold) {
                return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, dropItemId, 1);
            });
        }
    }
}

void Use_Item_On_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Use_Item_On_p::deserialize(): Received.");
    #endif
    deserializeVarInt(in_buff); // Hand: unused until multiple hands matter.
    DecodedPosition loc = DecodePosition(deserializeLong(in_buff));
    int face = deserializeVarInt(in_buff);
    deserializeFloat(in_buff); // Cursor Position X: unused, no sub-block placement logic yet.
    deserializeFloat(in_buff); // Cursor Position Y
    deserializeFloat(in_buff); // Cursor Position Z
    deserializeBool(in_buff);  // Inside block: unused.
    int sequence = deserializeVarInt(in_buff);

    int threshold = cont.connection.getCompressionThreshold();
    Player& player = cont.connection.getPlayer();
    const HotbarSlot& held = player.getHotbar()[player.getSelectedSlot()];
    Int32 blockStateId = itemIdToBlockStateId(held.itemId);
    if (blockStateId < 0) {
        // Unmapped/empty held item -- still ack (the client predicted a
        // placement that didn't happen), just no-op the world edit.
        cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
        return;
    }

    // Face -> outward normal (docs/network-protocol.md, "Player Action").
    int dx = 0, dy = 0, dz = 0;
    switch (face) {
        case 0: dy = -1; break; // Bottom
        case 1: dy = 1; break;  // Top
        case 2: dz = -1; break; // North
        case 3: dz = 1; break;  // South
        case 4: dx = -1; break; // West
        case 5: dx = 1; break;  // East
    }
    Int32 px = loc.x + dx, py = loc.y + dy, pz = loc.z + dz;

    World::getInstance().setBlock(px, py, pz, blockStateId);
    int chunkX = floorDiv16(px);
    int chunkZ = floorDiv16(pz);
    BroadcastToChunkViewers(chunkX, chunkZ, [px, py, pz, blockStateId](int broadcastThreshold) {
        return std::make_shared<Block_Update_p>(broadcastThreshold, px, py, pz, blockStateId);
    });
    cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
}

void Set_Held_Item_serverbound_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Held_Item_serverbound_p::deserialize(): Received.");
    #endif
    Int16 slot = deserializeShort(in_buff);
    cont.connection.getPlayer().setSelectedSlot(slot);
}