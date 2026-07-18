#include <Standards.hpp>
#include <Properties.hpp>
#include <network/packets/Play.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <network/Position.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <Player.hpp>
#include <World.hpp>
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
    // Encodes one section's Paletted Container. Builds a local palette by
    // first-seen order over the section's 4096 entries; a palette of size 1
    // (uniform section -- covers both "fully air" and "fully one solid
    // block") uses the cheap single-valued form (Bits Per Entry = 0). Anything
    // else uses the indirect palette form: Bits Per Entry = max(4, ceil(log2(paletteSize))),
    // a VarInt-prefixed palette of VarInt block-state IDs, then the per-block
    // palette indices packed into longs at that bit width (entriesPerLong =
    // 64/bits, filled LSB-first, never straddling a long boundary, remaining
    // high bits of a partially-filled last long left zero).
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

        // Even a single-valued (Bits Per Entry = 0) paletted container is still
        // followed by a Data Array Length VarInt -- always 0 here, since a single
        // value needs no per-block indices -- not omitted entirely. Verified against
        // decompiled 1.21 client bytecode: FriendlyByteBuf's long-array reader
        // unconditionally reads a VarInt length before reading any longs, and against
        // Pumpkin's (github.com/Pumpkin-MC/Pumpkin) working server implementation.
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
        // This generator's block variety (stone/dirt/grass/air) never comes
        // close to the 8-bit indirect ceiling, so the direct/global-palette
        // fallback isn't needed and isn't built.

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
}

Chunk_Data_p::Chunk_Data_p(int threshold, std::shared_ptr<Chunk> chunk) {
    _threshold = threshold;
    _chunk = chunk;
}

std::vector<Byte> Chunk_Data_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Chunk_Data_p::serialize(): Sending.");
    #endif
    const Int32 AIR_BLOCK_STATE_ID = 0; // stable across versions since the 1.13 flattening

    int chunkX = _chunk->getChunkX();
    int chunkZ = _chunk->getChunkZ();

    std::vector<Byte> packet_data;
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((chunkX >> (i * 8)) & 0xFF));
    for (int i = 3; i >= 0; i--) packet_data.push_back(static_cast<Byte>((chunkZ >> (i * 8)) & 0xFF));

    // Heightmaps: MOTION_BLOCKING, all-zero. Not yet computed from the chunk's
    // actual solid blocks (TODO once heightmaps matter for anything client-side
    // beyond rendering, e.g. random tick placement).
    // 256 columns at 9 bits/entry (ceil(log2(384+1))), 7 entries/long -> 37 longs, all zero.
    NbtTag heightmaps = NbtTag::makeCompound();
    heightmaps.put("MOTION_BLOCKING", NbtTag::makeLongArray(std::vector<Int64>(37, 0)));
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
        // A single count of non-empty blocks (blocks and fluids combined) --
        // there is only one count short on the wire, not separate block/fluid
        // counts (verified directly against decompiled 1.21 client bytecode,
        // LevelChunkSection's network read method: one readShort() then
        // straight into the block states container, no second short).
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

    // Lighting: 26 total light sections (Chunk::SECTION_COUNT real sections
    // plus one below-world and one above-world sentinel). Bit 0 = below-world,
    // bits 1..SECTION_COUNT = real sections in order, last bit = above-world
    // (docs/network-protocol.md). A section with all-zero light goes in the
    // "empty" mask (no array sent); anything else goes in the regular mask
    // with its packed array included, in ascending bit order.
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

    // Dispatch (and, in practice, deliver -- Connection::sendPackets() writes
    // each queued packet to the socket in order, one at a time, not batched)
    // nearest-to-center first. Otherwise, whenever a whole ring's worth of
    // chunks is ready simultaneously (e.g. all cache hits on a rejoin after
    // already exploring the area), the chunk directly under the player's own
    // feet -- needed before the client's "waiting for chunks" gate releases
    // gravity -- can end up serialized behind ~200 others, giving the client
    // long enough to free-fall before its own chunk ever arrives.
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