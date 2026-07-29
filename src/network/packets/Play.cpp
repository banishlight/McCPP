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
#include <BlockDropTable.hpp>
#include <ItemProperties.hpp>
#include <EntityIdAllocator.hpp>
#include <entities/ItemEntityManager.hpp>
#include <entities/FallingBlockEntityManager.hpp>
#include <entities/PlayerVisibilityManager.hpp>
#include <FluidBlocks.hpp>
#include <FluidUpdateQueue.hpp>
#include <CropBlocks.hpp>
#include <CropGrowthQueue.hpp>
#include <network/Crypto.hpp>
#include <network/Nbt.hpp>
#include <network/PlayerCommandSender.hpp>
#include <CommandRegistry.hpp>
#include <Console.hpp>
#include <sstream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <random>
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

Commands_p::Commands_p(int threshold, int permissionLevel) {
    _threshold = threshold;
    CommandRegistry& registry = CommandRegistry::getInstance();

    _nodes.push_back({0x00, {}, ""}); // root, always index 0
    std::vector<int> rootChildren; // accumulated locally -- _nodes.push_back below can reallocate, so a live reference into _nodes[0] isn't safe to hold across the loop

    for (const string& name : registry.getCommandNames()) {
        std::shared_ptr<Command> command = registry.getCommand(name);
        if (!command || permissionLevel < command->getRequiredPermission()) continue;

        std::vector<string> suggestions = command->getArgumentSuggestions();
        int commandNodeIndex = static_cast<int>(_nodes.size());
        // Executable on its own only if it takes no argument suggestions --
        // a command with suggestions isn't a complete invocation by itself
        // (e.g. bare "/gamemode" isn't valid without a mode).
        Byte commandFlags = static_cast<Byte>(0x01 | (suggestions.empty() ? 0x04 : 0x00));
        _nodes.push_back({commandFlags, {}, name});
        rootChildren.push_back(commandNodeIndex);

        // Literal-only suggestions (e.g. gamemode names) -- see
        // Command::getArgumentSuggestions and docs/general-documentation.md,
        // "Command autocomplete", for why this stops short of real Brigadier
        // argument/parser-ID encoding.
        for (const string& suggestion : suggestions) {
            int childIndex = static_cast<int>(_nodes.size());
            _nodes.push_back({static_cast<Byte>(0x01 | 0x04), {}, suggestion}); // literal, executable
            _nodes[commandNodeIndex].children.push_back(childIndex);
        }
    }

    _nodes[0].children = rootChildren;
}

std::vector<Byte> Commands_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Commands_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;

    std::vector<Byte> countBytes = varIntSerialize(static_cast<int>(_nodes.size()));
    packet_data.insert(packet_data.end(), countBytes.begin(), countBytes.end());

    for (const Node& node : _nodes) {
        packet_data.push_back(node.flags);
        std::vector<Byte> childCount = varIntSerialize(static_cast<int>(node.children.size()));
        packet_data.insert(packet_data.end(), childCount.begin(), childCount.end());
        for (int child : node.children) {
            std::vector<Byte> childIndexBytes = varIntSerialize(child);
            packet_data.insert(packet_data.end(), childIndexBytes.begin(), childIndexBytes.end());
        }
        if (!node.name.empty()) { // the root is the only node with no name
            std::vector<Byte> nameBytes = serializeString(node.name);
            packet_data.insert(packet_data.end(), nameBytes.begin(), nameBytes.end());
        }
    }

    std::vector<Byte> rootIndex = varIntSerialize(0); // root is node 0
    packet_data.insert(packet_data.end(), rootIndex.begin(), rootIndex.end());

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

Update_Time_p::Update_Time_p(int threshold, Int64 dayTime) {
    _threshold = threshold;
    _dayTime = dayTime;
}

std::vector<Byte> Update_Time_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Update_Time_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    // World Age, then Time of day -- both the same value here, since no
    // /time set command exists to ever diverge them.
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_dayTime >> (i * 8)) & 0xFF));
    }
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_dayTime >> (i * 8)) & 0xFF));
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

Sound_Effect_p::Sound_Effect_p(int threshold, Int32 soundId, int category, double x, double y, double z, float volume, float pitch, Int64 seed) {
    _threshold = threshold;
    _soundId = soundId;
    _category = category;
    _x = x;
    _y = y;
    _z = z;
    _volume = volume;
    _pitch = pitch;
    _seed = seed;
}

std::vector<Byte> Sound_Effect_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Sound_Effect_p::serialize(): Sending.");
    #endif
    // Sound ID + 1 -- the hardcoded numeric-id path. Sound Name/Has Fixed
    // Range/Range are only present when Sound ID is 0 (the identifier path),
    // which this project never uses.
    std::vector<Byte> packet_data = varIntSerialize(_soundId + 1);
    std::vector<Byte> categoryBytes = varIntSerialize(_category);
    packet_data.insert(packet_data.end(), categoryBytes.begin(), categoryBytes.end());

    // Effect Position X/Y/Z: fixed-point, position * 8, as a raw big-endian Int.
    Int32 fx = static_cast<Int32>(_x * 8.0);
    Int32 fy = static_cast<Int32>(_y * 8.0);
    Int32 fz = static_cast<Int32>(_z * 8.0);
    for (Int32 v : {fx, fy, fz}) {
        for (int i = 3; i >= 0; i--) {
            packet_data.push_back(static_cast<Byte>((v >> (i * 8)) & 0xFF));
        }
    }

    Int32 volumeBits;
    std::memcpy(&volumeBits, &_volume, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((volumeBits >> (i * 8)) & 0xFF));
    }
    Int32 pitchBits;
    std::memcpy(&pitchBits, &_pitch, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((pitchBits >> (i * 8)) & 0xFF));
    }

    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_seed >> (i * 8)) & 0xFF));
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

    // Slot (1.20.5+ Data Components format, verified since
    // docs/network-protocol.md only links out to an external page for this
    // type): empty is a single VarInt(0). Non-empty is VarInt(count),
    // VarInt(item ID), then VarInt(components-to-add count) and
    // VarInt(components-to-remove count) -- both 0 here, since nothing this
    // server places needs a non-default component (custom name, damage,
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

    // Mirror of packSlot, for the one incoming packet that needs it
    // (Set_Creative_Mode_Slot_p). count == 0 means empty. A nonzero
    // addComponents/removeComponents count means the item carries real
    // component data (enchantments, custom name, etc.) this project has no
    // model for and never produces itself -- rather than parse variable-
    // length, type-specific component payloads, that case is reported back
    // as "unsupported" (present=false) via the count=-1 sentinel, and the
    // caller no-ops. Safe to do: each packet's own outer length prefix
    // already demarcates its exact byte range before dispatch, so not fully
    // consuming this one packet's payload can't desync anything after it.
    struct DecodedSlot {
        bool present;
        Int32 itemId;
        Int32 count;
    };
    DecodedSlot unpackSlot(std::vector<Byte>& in_buff) {
        Int32 count = deserializeVarInt(in_buff);
        if (count == 0) return {false, -1, 0};
        Int32 itemId = deserializeVarInt(in_buff);
        Int32 addCount = deserializeVarInt(in_buff);
        Int32 removeCount = deserializeVarInt(in_buff);
        if (addCount != 0 || removeCount != 0) return {false, -1, -1};
        return {true, itemId, count};
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

Set_Container_Content_p::Set_Container_Content_p(int threshold, const std::array<InventorySlot, Player::TOTAL_SLOTS>& inventory, int stateId, const InventorySlot& carriedItem) {
    _threshold = threshold;
    _inventory = inventory;
    _stateId = stateId;
    _carriedItem = carriedItem;
}

std::vector<Byte> Set_Container_Content_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Container_Content_p::serialize(): Sending.");
    #endif
    // Player inventory (Window ID 0) container-slot layout: 0 = crafting
    // result, 1-4 = crafting grid, 5-8 = armor, 9-35 = main inventory,
    // 36-44 = hotbar, 45 = offhand -- see Player.hpp's TOTAL_SLOTS/
    // HOTBAR_START comment, the same layout Player's own array uses, so no
    // hotbar-relative translation is needed here anymore.
    std::vector<Byte> packet_data;
    packet_data.push_back(0x00); // Window ID: 0 = player inventory
    std::vector<Byte> stateId = varIntSerialize(_stateId);
    packet_data.insert(packet_data.end(), stateId.begin(), stateId.end());
    std::vector<Byte> count = varIntSerialize(Player::TOTAL_SLOTS);
    packet_data.insert(packet_data.end(), count.begin(), count.end());
    for (int i = 0; i < Player::TOTAL_SLOTS; i++) {
        std::vector<Byte> slotBytes = packSlot(_inventory[i].itemId, _inventory[i].count);
        packet_data.insert(packet_data.end(), slotBytes.begin(), slotBytes.end());
    }
    std::vector<Byte> carried = packSlot(_carriedItem.itemId, _carriedItem.count);
    packet_data.insert(packet_data.end(), carried.begin(), carried.end());

    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Container_Slot_p::Set_Container_Slot_p(int threshold, int windowId, int stateId, int slotIndex, Int32 itemId, Int32 count) {
    _threshold = threshold;
    _windowId = windowId;
    _stateId = stateId;
    _slotIndex = slotIndex;
    _itemId = itemId;
    _count = count;
}

std::vector<Byte> Set_Container_Slot_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Container_Slot_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    packet_data.push_back(static_cast<Byte>(_windowId)); // -1 = the carried/dragged item, matches vanilla's own convention
    std::vector<Byte> stateId = varIntSerialize(_stateId);
    packet_data.insert(packet_data.end(), stateId.begin(), stateId.end());
    Int16 slot16 = static_cast<Int16>(_slotIndex);
    packet_data.push_back(static_cast<Byte>((slot16 >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(slot16 & 0xFF));
    std::vector<Byte> slotBytes = packSlot(_itemId, _count);
    packet_data.insert(packet_data.end(), slotBytes.begin(), slotBytes.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

Spawn_Entity_p::Spawn_Entity_p(int threshold, int entityId, const std::vector<long>& uuid, int entityTypeId, double x, double y, double z,
                               float yaw, float pitch, float headYaw, Int32 data) {
    _threshold = threshold;
    _entityId = entityId;
    _uuid = uuid;
    _entityTypeId = entityTypeId;
    _x = x;
    _y = y;
    _z = z;
    _yaw = yaw;
    _pitch = pitch;
    _headYaw = headYaw;
    _data = data;
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

    packet_data.push_back(angleSerialize(_pitch));
    packet_data.push_back(angleSerialize(_yaw));
    packet_data.push_back(angleSerialize(_headYaw));

    std::vector<Byte> dataBytes = varIntSerialize(_data); // Object Data: meaning depends on entity type, see docs/general-documentation.md
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
    // type 7. -- see docs/general-documentation.md.
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

Set_Player_Skin_Parts_Metadata_p::Set_Player_Skin_Parts_Metadata_p(int threshold, int entityId, Byte skinParts) {
    _threshold = threshold;
    _entityId = entityId;
    _skinParts = skinParts;
}

std::vector<Byte> Set_Player_Skin_Parts_Metadata_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Player_Skin_Parts_Metadata_p::serialize(): Sending.");
    #endif
    // Displayed Skin Parts bitmask (cape/jacket/sleeves/pants/hat), type 0 =
    // Byte. Index 17 confirmed against an archived minecraft.wiki revision
    // explicitly labeled "Java Edition 1.20.2" (this field is stable across
    // adjacent versions; the *live* wiki page shows 16 instead, but that
    // reflects a much newer snapshot, not 1.21).
    const Byte PLAYER_SKIN_PARTS_METADATA_INDEX = 17;
    const int BYTE_METADATA_TYPE = 0;

    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(PLAYER_SKIN_PARTS_METADATA_INDEX);
    std::vector<Byte> typeBytes = varIntSerialize(BYTE_METADATA_TYPE);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());
    packet_data.push_back(_skinParts);
    packet_data.push_back(0xFF); // terminator: no more metadata entries

    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Entity_Flags_Metadata_p::Set_Entity_Flags_Metadata_p(int threshold, int entityId, bool sneaking, bool sprinting) {
    _threshold = threshold;
    _entityId = entityId;
    _sneaking = sneaking;
    _sprinting = sprinting;
}

std::vector<Byte> Set_Entity_Flags_Metadata_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Entity_Flags_Metadata_p::serialize(): Sending.");
    #endif
    // Entity Flags, index 0, type 0/Byte: bit 0x02 crouching, bit 0x08 sprinting.
    const Byte ENTITY_FLAGS_INDEX = 0;
    const int BYTE_METADATA_TYPE = 0;

    Byte flags = 0;
    if (_sneaking) flags |= 0x02;
    if (_sprinting) flags |= 0x08;

    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(ENTITY_FLAGS_INDEX);
    std::vector<Byte> typeBytes = varIntSerialize(BYTE_METADATA_TYPE);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());
    packet_data.push_back(flags);
    packet_data.push_back(0xFF); // terminator: no more metadata entries
    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Player_Pose_Metadata_p::Set_Player_Pose_Metadata_p(int threshold, int entityId, int pose) {
    _threshold = threshold;
    _entityId = entityId;
    _pose = pose;
}

std::vector<Byte> Set_Player_Pose_Metadata_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Player_Pose_Metadata_p::serialize(): Sending.");
    #endif
    // Pose, index 6. Its value is wire-identical to a plain VarInt, but Pose
    // has its own dedicated metadata type ID distinct from the generic VarInt
    // type (1) -- using 1 silently failed even though the value bytes
    // themselves would have been identical either way. That type ID is ALSO
    // version-specific: it's 21 for 1.21 through 1.21.7, shifting to 20 only
    // at 1.21.9+ (and renamed again in later snapshots) -- verified against
    // version-pinned data after the type-registry table on the live wiki page
    // (which defaults to showing the newest version, same trap as the
    // skin-parts index) gave 20 and silently failed for our actual 1.21 target.
    const Byte POSE_INDEX = 6;
    const int POSE_METADATA_TYPE = 21;

    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(POSE_INDEX);
    std::vector<Byte> typeBytes = varIntSerialize(POSE_METADATA_TYPE);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());
    std::vector<Byte> valueBytes = varIntSerialize(_pose);
    packet_data.insert(packet_data.end(), valueBytes.begin(), valueBytes.end());
    packet_data.push_back(0xFF); // terminator: no more metadata entries
    return assemblePacket(getID(), _threshold, packet_data);
}

Remove_Entities_p::Remove_Entities_p(int threshold, int entityId) {
    _threshold = threshold;
    _entityId = entityId;
}

std::vector<Byte> Remove_Entities_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Remove_Entities_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(1); // Count: always a single entity here
    std::vector<Byte> idBytes = varIntSerialize(_entityId);
    packet_data.insert(packet_data.end(), idBytes.begin(), idBytes.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

Pickup_Item_p::Pickup_Item_p(int threshold, int collectedEntityId, int collectorEntityId, int count) {
    _threshold = threshold;
    _collectedEntityId = collectedEntityId;
    _collectorEntityId = collectorEntityId;
    _count = count;
}

std::vector<Byte> Pickup_Item_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Pickup_Item_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_collectedEntityId);
    std::vector<Byte> collectorBytes = varIntSerialize(_collectorEntityId);
    packet_data.insert(packet_data.end(), collectorBytes.begin(), collectorBytes.end());
    std::vector<Byte> countBytes = varIntSerialize(_count);
    packet_data.insert(packet_data.end(), countBytes.begin(), countBytes.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Entity_Velocity_p::Set_Entity_Velocity_p(int threshold, int entityId, double vx, double vy, double vz) {
    _threshold = threshold;
    _entityId = entityId;
    _vx = vx;
    _vy = vy;
    _vz = vz;
}

std::vector<Byte> Set_Entity_Velocity_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Entity_Velocity_p::serialize(): Sending.");
    #endif
    // 1/8000 of a block per tick, per docs/network-protocol.md.
    Int16 wireVx = static_cast<Int16>(_vx * 8000.0);
    Int16 wireVy = static_cast<Int16>(_vy * 8000.0);
    Int16 wireVz = static_cast<Int16>(_vz * 8000.0);

    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(static_cast<Byte>((wireVx >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(wireVx & 0xFF));
    packet_data.push_back(static_cast<Byte>((wireVy >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(wireVy & 0xFF));
    packet_data.push_back(static_cast<Byte>((wireVz >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(wireVz & 0xFF));
    return assemblePacket(getID(), _threshold, packet_data);
}

Teleport_Entity_p::Teleport_Entity_p(int threshold, int entityId, double x, double y, double z, float yaw, float pitch, bool onGround) {
    _threshold = threshold;
    _entityId = entityId;
    _x = x;
    _y = y;
    _z = z;
    _yaw = yaw;
    _pitch = pitch;
    _onGround = onGround;
}

std::vector<Byte> Teleport_Entity_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Teleport_Entity_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);

    Int64 xBits, yBits, zBits;
    std::memcpy(&xBits, &_x, sizeof(Int64));
    std::memcpy(&yBits, &_y, sizeof(Int64));
    std::memcpy(&zBits, &_z, sizeof(Int64));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((xBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((yBits >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; i--) packet_data.push_back(static_cast<Byte>((zBits >> (i * 8)) & 0xFF));

    packet_data.push_back(angleSerialize(_yaw));
    packet_data.push_back(angleSerialize(_pitch));
    packet_data.push_back(_onGround ? 0x01 : 0x00);
    return assemblePacket(getID(), _threshold, packet_data);
}

Update_Entity_Position_p::Update_Entity_Position_p(int threshold, int entityId, Int16 deltaX, Int16 deltaY, Int16 deltaZ, bool onGround) {
    _threshold = threshold;
    _entityId = entityId;
    _deltaX = deltaX;
    _deltaY = deltaY;
    _deltaZ = deltaZ;
    _onGround = onGround;
}

std::vector<Byte> Update_Entity_Position_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Update_Entity_Position_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(static_cast<Byte>((_deltaX >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaX & 0xFF));
    packet_data.push_back(static_cast<Byte>((_deltaY >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaY & 0xFF));
    packet_data.push_back(static_cast<Byte>((_deltaZ >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaZ & 0xFF));
    packet_data.push_back(_onGround ? 0x01 : 0x00);
    return assemblePacket(getID(), _threshold, packet_data);
}

Update_Entity_Position_and_Rotation_p::Update_Entity_Position_and_Rotation_p(int threshold, int entityId, Int16 deltaX, Int16 deltaY, Int16 deltaZ, float yaw, float pitch, bool onGround) {
    _threshold = threshold;
    _entityId = entityId;
    _deltaX = deltaX;
    _deltaY = deltaY;
    _deltaZ = deltaZ;
    _yaw = yaw;
    _pitch = pitch;
    _onGround = onGround;
}

std::vector<Byte> Update_Entity_Position_and_Rotation_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Update_Entity_Position_and_Rotation_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(static_cast<Byte>((_deltaX >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaX & 0xFF));
    packet_data.push_back(static_cast<Byte>((_deltaY >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaY & 0xFF));
    packet_data.push_back(static_cast<Byte>((_deltaZ >> 8) & 0xFF));
    packet_data.push_back(static_cast<Byte>(_deltaZ & 0xFF));
    packet_data.push_back(angleSerialize(_yaw));
    packet_data.push_back(angleSerialize(_pitch));
    packet_data.push_back(_onGround ? 0x01 : 0x00);
    return assemblePacket(getID(), _threshold, packet_data);
}

Update_Entity_Rotation_p::Update_Entity_Rotation_p(int threshold, int entityId, float yaw, float pitch, bool onGround) {
    _threshold = threshold;
    _entityId = entityId;
    _yaw = yaw;
    _pitch = pitch;
    _onGround = onGround;
}

std::vector<Byte> Update_Entity_Rotation_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Update_Entity_Rotation_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(angleSerialize(_yaw));
    packet_data.push_back(angleSerialize(_pitch));
    packet_data.push_back(_onGround ? 0x01 : 0x00);
    return assemblePacket(getID(), _threshold, packet_data);
}

Set_Head_Rotation_p::Set_Head_Rotation_p(int threshold, int entityId, float headYaw) {
    _threshold = threshold;
    _entityId = entityId;
    _headYaw = headYaw;
}

std::vector<Byte> Set_Head_Rotation_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Head_Rotation_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(_entityId);
    packet_data.push_back(angleSerialize(_headYaw));
    return assemblePacket(getID(), _threshold, packet_data);
}

Player_Info_Update_p::Player_Info_Update_p(int threshold, const std::vector<Entry>& entries) {
    _threshold = threshold;
    _entries = entries;
}

std::vector<Byte> Player_Info_Update_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Player_Info_Update_p::serialize(): Sending.");
    #endif
    // Add Player (0x01) | Update Game Mode (0x04) | Update Listed (0x08) --
    // Update Listed is the one that actually makes a player appear in the
    // client's tab-list overlay at all; without it, the client defaults an
    // entry to unlisted (an empty-looking overlay), even though Add Player's
    // own fields (name/skin) are otherwise applied fine.
    const Byte ACTIONS = 0x01 | 0x04 | 0x08;
    std::vector<Byte> packet_data;
    packet_data.push_back(ACTIONS);
    std::vector<Byte> count = varIntSerialize(static_cast<int>(_entries.size()));
    packet_data.insert(packet_data.end(), count.begin(), count.end());

    // Interleaved per player (UUID immediately followed by that player's own
    // action fields, in the same order as the Actions bitmask's set bits: Add
    // Player, then Update Game Mode, then Update Listed), not all UUIDs then
    // all action blocks -- the vendored doc's table is ambiguous on grouping,
    // verified rather than guessed.
    for (const auto& entry : _entries) {
        std::vector<Byte> uuidBytes = serializeUUID(entry.uuid);
        packet_data.insert(packet_data.end(), uuidBytes.begin(), uuidBytes.end());

        std::vector<Byte> nameBytes = serializeString(entry.name);
        packet_data.insert(packet_data.end(), nameBytes.begin(), nameBytes.end());
        std::vector<Byte> propCount = varIntSerialize(static_cast<int>(entry.properties.size()));
        packet_data.insert(packet_data.end(), propCount.begin(), propCount.end());
        for (const auto& prop : entry.properties) {
            std::vector<Byte> nameB = serializeString(prop.name);
            std::vector<Byte> valueB = serializeString(prop.value);
            packet_data.insert(packet_data.end(), nameB.begin(), nameB.end());
            packet_data.insert(packet_data.end(), valueB.begin(), valueB.end());
            if (!prop.signature.empty()) {
                packet_data.push_back(0x01);
                std::vector<Byte> sigB = serializeString(prop.signature);
                packet_data.insert(packet_data.end(), sigB.begin(), sigB.end());
            } else {
                packet_data.push_back(0x00);
            }
        }

        std::vector<Byte> gamemodeBytes = varIntSerialize(entry.gamemode);
        packet_data.insert(packet_data.end(), gamemodeBytes.begin(), gamemodeBytes.end());

        packet_data.push_back(0x01); // Listed = true, always -- no hide-from-tab-list mechanism exists
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

Player_Info_Remove_p::Player_Info_Remove_p(int threshold, const std::vector<std::vector<long>>& uuids) {
    _threshold = threshold;
    _uuids = uuids;
}

std::vector<Byte> Player_Info_Remove_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Player_Info_Remove_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = varIntSerialize(static_cast<int>(_uuids.size()));
    for (const auto& uuid : _uuids) {
        std::vector<Byte> uuidBytes = serializeUUID(uuid);
        packet_data.insert(packet_data.end(), uuidBytes.begin(), uuidBytes.end());
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

namespace {
    // {"text": value} as network NBT -- the simplest valid text component,
    // sufficient since this server never generates colored/translated chat.
    std::vector<Byte> makeTextComponentNbt(const string& value) {
        NbtTag comp = NbtTag::makeCompound();
        comp.put("text", NbtTag::makeString(value));
        return comp.serializeNetwork();
    }
}

Disconnect_play_p::Disconnect_play_p(int threshold, const string& reason) {
    _threshold = threshold;
    _reason = reason;
}

std::vector<Byte> Disconnect_play_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Disconnect_play_p::serialize(): Sending.");
    #endif
    return assemblePacket(getID(), _threshold, makeTextComponentNbt(_reason));
}

Disguised_Chat_Message_p::Disguised_Chat_Message_p(int threshold, const string& message, int chatTypeIndex, const string& senderName) {
    _threshold = threshold;
    _message = message;
    _chatTypeIndex = chatTypeIndex;
    _senderName = senderName;
}

std::vector<Byte> Disguised_Chat_Message_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Disguised_Chat_Message_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = makeTextComponentNbt(_message);
    // Chat Type is a Holder<ChatType>, not a plain registry index: the wire
    // value is (index + 1), with 0 reserved to mean "an inline value follows"
    // instead of a registry reference. Not mentioned by the vendored doc's
    // plain "VarInt" field type -- sending the raw index made the client read
    // 0 as "inline value" and try to parse Sender Name's NBT as a ChatType
    // definition, which is what a "Failed to decode disguised_chat" turned
    // out to be.
    std::vector<Byte> typeBytes = varIntSerialize(_chatTypeIndex + 1);
    packet_data.insert(packet_data.end(), typeBytes.begin(), typeBytes.end());
    std::vector<Byte> senderBytes = makeTextComponentNbt(_senderName);
    packet_data.insert(packet_data.end(), senderBytes.begin(), senderBytes.end());
    packet_data.push_back(0x00); // Has Target Name: always false, no /tell support.
    return assemblePacket(getID(), _threshold, packet_data);
}

System_Chat_Message_p::System_Chat_Message_p(int threshold, const string& message, bool overlay) {
    _threshold = threshold;
    _message = message;
    _overlay = overlay;
}

std::vector<Byte> System_Chat_Message_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("System_Chat_Message_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data = makeTextComponentNbt(_message);
    packet_data.push_back(_overlay ? 0x01 : 0x00);
    return assemblePacket(getID(), _threshold, packet_data);
}

void BroadcastDisguisedChat(const string& senderName, const string& message, const string& chatTypeId) {
    // chat_type registry send order isn't fixed (directory_iterator order),
    // so the wire index has to be looked up by id, same pattern Login_Play_p
    // uses for dimension_type.
    int chatTypeIndex = 0;
    const std::vector<RegistryEntry>& chatTypes = VanillaDataManager::getInstance().getEntries("chat_type");
    for (size_t i = 0; i < chatTypes.size(); i++) {
        if (chatTypes[i].id == chatTypeId) {
            chatTypeIndex = static_cast<int>(i);
            break;
        }
    }

    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& conn : connections) {
        if (!conn) continue;
        if (conn->getState() != ConnectionState::Play) continue;
        int threshold = conn->getCompressionThreshold();
        conn->addPacket(std::make_shared<Disguised_Chat_Message_p>(threshold, message, chatTypeIndex, senderName));
    }
}

Byte abilitiesFlagsForGamemode(int gamemode) {
    const Byte INVULNERABLE = 0x01;
    const Byte FLYING = 0x02;
    const Byte ALLOW_FLYING = 0x04;
    const Byte INSTANT_BREAK = 0x08;
    switch (gamemode) {
        case 1: return INVULNERABLE | ALLOW_FLYING | INSTANT_BREAK; // Creative
        case 3: return INVULNERABLE | FLYING | ALLOW_FLYING; // Spectator -- always flies/noclips
        default: return 0x00; // Survival, Adventure
    }
}

Player_Abilities_p::Player_Abilities_p(int threshold, Byte flags, float flyingSpeed, float fovModifier) {
    _threshold = threshold;
    _flags = flags;
    _flyingSpeed = flyingSpeed;
    _fovModifier = fovModifier;
}

std::vector<Byte> Player_Abilities_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Player_Abilities_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    packet_data.push_back(_flags);

    Int32 speedBits;
    std::memcpy(&speedBits, &_flyingSpeed, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((speedBits >> (i * 8)) & 0xFF));
    }

    Int32 fovBits;
    std::memcpy(&fovBits, &_fovModifier, sizeof(Int32));
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((fovBits >> (i * 8)) & 0xFF));
    }

    return assemblePacket(getID(), _threshold, packet_data);
}

void BroadcastPlayerJoin(PacketContext& cont, Player& joiningPlayer) {
    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();

    Player_Info_Update_p::Entry joiningEntry;
    joiningEntry.uuid = joiningPlayer.getUUID();
    joiningEntry.name = joiningPlayer.getUsername();
    joiningEntry.properties = joiningPlayer.getProfileProperties();
    joiningEntry.gamemode = joiningPlayer.getGamemode();

    // The client needs its own Add Player entry too, not just other players' --
    // skin variant (slim/classic arms) and cape data are only ever delivered via
    // this packet, and third-person rendering of yourself depends on it just
    // like it does for everyone else.
    std::vector<Player_Info_Update_p::Entry> entriesForJoiner{joiningEntry};

    for (auto& conn : connections) {
        if (!conn) continue;
        if (conn.get() == &cont.connection) continue; // the joining connection itself
        if (conn->getState() != ConnectionState::Play) continue;

        int otherThreshold = conn->getCompressionThreshold();
        conn->addPacket(std::make_shared<Player_Info_Update_p>(otherThreshold, std::vector<Player_Info_Update_p::Entry>{joiningEntry}));

        Player& other = conn->getPlayer();
        Player_Info_Update_p::Entry existingEntry;
        existingEntry.uuid = other.getUUID();
        existingEntry.name = other.getUsername();
        existingEntry.properties = other.getProfileProperties();
        existingEntry.gamemode = other.getGamemode();
        entriesForJoiner.push_back(existingEntry);
    }

    int threshold = cont.connection.getCompressionThreshold();
    cont.connection.addPacket(std::make_shared<Player_Info_Update_p>(threshold, entriesForJoiner));
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

namespace {
    // Shared by CheckPlantSupport and ResolveFluid's plant/crop destruction --
    // the same spawn-a-real-pickup-able-entity-and-broadcast-it sequence the
    // break handler uses, factored out now that it's needed in more than one
    // place (the break handler's own two call sites are older/already-tested
    // code, left as-is rather than churned for this).
    void SpawnDroppedItem(int chunkX, int chunkZ, double dropX, double dropY, double dropZ, Int32 itemId, Int32 count) {
        const int ITEM_ENTITY_TYPE_ID = 58;
        ItemEntity dropped = ItemEntityManager::getInstance().spawn(itemId, count, dropX, dropY, dropZ, chunkX, chunkZ);
        std::vector<long> uuid = generateRandomUUID(); // one-time, not persisted -- only needed for this Spawn_Entity_p
        int entityId = dropped.entityId;
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, dropX, dropY, dropZ, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
            return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, dropX, dropY, dropZ);
        });
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, itemId, count](int broadcastThreshold) {
            return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, itemId, count);
        });
    }

    // The one real drop-resolution sequence a break (of any origin -- player,
    // lost support, fluid sweep) should go through: BlockDropTable's override
    // table first, falling back to "drops itself" when there's no entry, plus
    // whatever independent bonus stack (mature wheat/beetroot's bonus seeds)
    // applies. Always with a default (no Fortune/Silk Touch) ToolInfo -- none
    // of these call sites represent an actual player-held tool.
    void SpawnBlockBreakDrops(int x, int y, int z, int chunkX, int chunkZ, Int32 blockStateId) {
        BlockDropTable::ToolInfo tool;
        Int32 dropItemId, dropCount;
        if (!BlockDropTable::CheckDrop(blockStateId, tool, dropItemId, dropCount)) {
            dropItemId = blockStateIdToItemId(blockStateId);
            dropCount = 1;
        }
        Int32 bonusDropItemId = -1, bonusDropCount = 0;
        BlockDropTable::CheckBonusDrop(blockStateId, tool, bonusDropItemId, bonusDropCount);

        double dropX = x + 0.5, dropY = y + 0.5, dropZ = z + 0.5;
        if (dropItemId >= 0 && dropCount > 0) {
            SpawnDroppedItem(chunkX, chunkZ, dropX, dropY, dropZ, dropItemId, dropCount);
        }
        if (bonusDropItemId >= 0 && bonusDropCount > 0) {
            SpawnDroppedItem(chunkX, chunkZ, dropX, dropY, dropZ, bonusDropItemId, bonusDropCount);
        }
    }
}

void CheckGravityBlock(World& world, int x, int y, int z) {
    // Above the build limit -- Chunk's flat array has no room past WORLD_HEIGHT,
    // and there's nothing there anyway. Below the floor never happens (callers
    // only ever pass a position that just held or gained a real block).
    if (y < Chunk::WORLD_MIN_Y || y >= Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT) return;

    int chunkX = floorDiv16(x);
    int chunkZ = floorDiv16(z);
    std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
    if (!chunk) return;

    int localX = x - chunkX * 16;
    int localZ = z - chunkZ * 16;
    Int32 blockId = chunk->getBlock(localX, y, localZ);
    if (blockId != SAND_BLOCK_STATE_ID && blockId != GRAVEL_BLOCK_STATE_ID) return;

    // World floor: treated as solid support, matching ItemPhysicsSystem's own
    // "can't check below the bottom of the world" stance.
    if (y <= Chunk::WORLD_MIN_Y) return;
    Int32 below = chunk->getBlock(localX, y - 1, localZ);
    if (below != AIR_BLOCK_STATE_ID) return; // supported, nothing to do

    // Unsupported: pull it out of the static world and hand it off to
    // FallingBlockSystem as a real, simulated entity.
    world.setBlock(x, y, z, AIR_BLOCK_STATE_ID);
    BroadcastToChunkViewers(chunkX, chunkZ, [x, y, z](int broadcastThreshold) {
        return std::make_shared<Block_Update_p>(broadcastThreshold, x, y, z, AIR_BLOCK_STATE_ID);
    });

    FallingBlockEntity falling = FallingBlockEntityManager::getInstance().spawn(blockId, x + 0.5, y, z + 0.5, chunkX, chunkZ);
    std::vector<long> uuid = generateRandomUUID(); // one-time, not persisted -- only needed for this Spawn_Entity_p
    int entityId = falling.entityId;
    // minecraft:falling_block entity-type registry ID, sourced from the
    // vanilla data generator report, not guessed -- cross-checked against the
    // already-known item (58) and player (128) entity-type IDs in the same report.
    const int FALLING_BLOCK_ENTITY_TYPE_ID = 40;
    BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, x, y, z, blockId, FALLING_BLOCK_ENTITY_TYPE_ID](int broadcastThreshold) {
        // Object Data carries the block state ID for this entity type (confirmed
        // against a version-pinned minecraft.wiki revision, not the live page) --
        // no metadata packet is needed, unlike item entities: Falling Block's
        // only metadata field (index 8, "spawn position") is optional and this
        // project never sends optional fields it has no use for.
        return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, FALLING_BLOCK_ENTITY_TYPE_ID,
                                                 x + 0.5, y, z + 0.5, 0.0f, 0.0f, 0.0f, blockId);
    });

    // Cascade: the position that just became air might itself have been
    // supporting another gravity block directly above it.
    CheckGravityBlock(world, x, y + 1, z);
    // ...or a plant (grass/flowers need solid ground, same as sand needs air
    // below it -- both are "does my support still exist" checks).
    CheckPlantSupport(world, x, y + 1, z);
}

// Short grass/poppy/dandelion (the only 3 non-solid "plant" blocks this
// project places, see BlockIds.hpp) require a real block directly beneath
// them, same as real vanilla -- if that support disappears (broken by a
// player, or exposed by a gravity block falling out from under it), the
// plant pops off exactly like a direct player break would: real drop table
// lookup, real dropped-item entity, not silently deleted.
void CheckPlantSupport(World& world, int x, int y, int z) {
    if (y < Chunk::WORLD_MIN_Y || y >= Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT) return;

    int chunkX = floorDiv16(x);
    int chunkZ = floorDiv16(z);
    std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
    if (!chunk) return;

    int localX = x - chunkX * 16;
    int localZ = z - chunkZ * 16;
    Int32 blockId = chunk->getBlock(localX, y, localZ);
    bool isPlant = (blockId == SHORT_GRASS_STATE_ID || blockId == POPPY_STATE_ID || blockId == DANDELION_STATE_ID);
    if (!isPlant) return;

    if (y <= Chunk::WORLD_MIN_Y) return; // world floor counts as solid support
    Int32 below = chunk->getBlock(localX, y - 1, localZ);
    // Supported by anything solid; unsupported if it's air OR fluid just
    // flowed/fell into that spot (a plant can't stand on open water either).
    if (below != AIR_BLOCK_STATE_ID && Fluid::typeOf(below) == Fluid::Type::None) return;

    world.setBlock(x, y, z, AIR_BLOCK_STATE_ID);
    BroadcastToChunkViewers(chunkX, chunkZ, [x, y, z](int broadcastThreshold) {
        return std::make_shared<Block_Update_p>(broadcastThreshold, x, y, z, AIR_BLOCK_STATE_ID);
    });
    SpawnBlockBreakDrops(x, y, z, chunkX, chunkZ, blockId);

    // Cascade: this plant's own position just became air -- something might
    // have been resting directly on top of IT too (another plant, or a
    // gravity block that was, unrealistically but pre-existing, treated as
    // supported by a non-solid plant beneath it).
    CheckGravityBlock(world, x, y + 1, z);
    CheckPlantSupport(world, x, y + 1, z);
}

namespace {
    // -1 signals "chunk not loaded / out of world bounds" -- distinct from
    // AIR_BLOCK_STATE_ID (0). Callers must treat it as "unknown, don't touch"
    // rather than as air, matching CheckGravityBlock's own stance on unloaded
    // chunks elsewhere in this file.
    Int32 GetWorldBlockOrUnknown(World& world, int x, int y, int z) {
        if (y < Chunk::WORLD_MIN_Y || y >= Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT) return -1;
        int chunkX = floorDiv16(x);
        int chunkZ = floorDiv16(z);
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) return -1;
        return chunk->getBlock(x - chunkX * 16, y, z - chunkZ * 16);
    }

    // Vanilla's real Overworld rates -- water re-checks every 5 ticks, lava
    // every 30 -- confirmed against minecraft.wiki, not guessed. This project
    // has no per-dimension distinction, so there's no separate Nether rate to
    // pick between.
    int FluidTickInterval(Fluid::Type type) {
        return (type == Fluid::Type::Lava) ? 30 : 5;
    }

    void ScheduleFluidCheck(int x, int y, int z, int delayTicks) {
        FluidUpdateQueue::getInstance().schedule(x, y, z, delayTicks);
    }

    // Schedules a position and its 6 neighbors -- used both for the initial
    // "something changed nearby" kick-off (see ScheduleFluidNeighbors's own
    // call sites in the break/place handlers, delay 1) and by ResolveFluid
    // itself once it determines a position's correct fluid-specific pace.
    void ScheduleFluidNeighbors(int x, int y, int z, int delayTicks) {
        ScheduleFluidCheck(x, y, z, delayTicks);
        ScheduleFluidCheck(x + 1, y, z, delayTicks);
        ScheduleFluidCheck(x - 1, y, z, delayTicks);
        ScheduleFluidCheck(x, y, z + 1, delayTicks);
        ScheduleFluidCheck(x, y, z - 1, delayTicks);
        ScheduleFluidCheck(x, y + 1, z, delayTicks);
        ScheduleFluidCheck(x, y - 1, z, delayTicks);
    }
}

namespace {
    struct FluidSupply {
        Fluid::Type type = Fluid::Type::None;
        bool falling = false;
        int distance = 0; // meaningful only when type != None && !falling
    };

    // Finds the TRUE shortest distance-equivalent cost from (x,y,z) to an
    // actual source of `type` (or a column fed from directly above),
    // searching outward through a chain of "capped" same-type fluid tiles.
    // Each hop adds `Fluid::levelDecreasePerBlock(type)` to the running cost
    // rather than a flat 1 -- water and lava do NOT decay at the same rate:
    // lava's cost doubles per block, capping its reach at 3 blocks instead of
    // water's 7. A first version of this treated both fluids
    // identically, which meant lava was assigned level values (1, 3, 5, 7)
    // real lava never produces -- only even levels (2, 4, 6) are reachable
    // with a decrease-per-block of 2 -- and the client, having no precedent
    // for those states on an actual lava block, rendered them wrong (this is
    // what a user-reported "lava looks like water" bug turned out to be, not
    // a wire-encoding problem -- the encoding itself was independently
    // verified correct, see docs).
    //
    // Deliberately does NOT trust any intermediate tile's own claimed
    // distance -- only whether it's fluid and capped, i.e. eligible as a
    // pass-through link -- and instead computes the real cost to a genuine
    // anchor itself. A single-hop "peek at my neighbor's stored distance and
    // add one" version of this was a separate real, reported bug: two
    // neighboring tiles whose shared source had just been removed could
    // reference each other's still-present (but equally orphaned) values and
    // prop each other up, so an entire pool would decay in lockstep and
    // vanish in one layer instead of retreating outward the same way it grew
    // -- and a freshly-vacated tile could likewise "regrow" from a neighbor
    // that was itself about to be invalidated. Recomputing from real anchors
    // every time sidesteps both: a tile with no genuine path to a source
    // correctly finds nothing, regardless of what stale distance values
    // happen to be sitting on nearby not-yet-resolved tiles.
    FluidSupply FindFluidSupplyForType(World& world, int x, int y, int z, Fluid::Type type) {
        Int32 above = GetWorldBlockOrUnknown(world, x, y + 1, z);
        if (Fluid::typeOf(above) == type) {
            return {type, true, 0};
        }

        int perBlock = Fluid::levelDecreasePerBlock(type);
        struct Node { int x, z, cost; };
        std::vector<Node> queue;
        std::set<std::pair<int,int>> visited;
        queue.push_back({x, z, 0});
        visited.insert({x, z});

        Fluid::Type foundType = Fluid::Type::None;
        int foundCost = 8; // sentinel: worse than any real 1-7 cost

        static constexpr int dx[4] = {1, -1, 0, 0};
        static constexpr int dz[4] = {0, 0, 1, -1};
        for (size_t head = 0; head < queue.size(); head++) {
            Node node = queue[head];
            for (int i = 0; i < 4; i++) {
                int nx = node.x + dx[i], nz = node.z + dz[i];
                if (!visited.insert({nx, nz}).second) continue;

                Int32 neighbor = GetWorldBlockOrUnknown(world, nx, y, nz);
                if (Fluid::typeOf(neighbor) != type) continue;

                int newCost = node.cost + perBlock;
                if (Fluid::isSource(neighbor)) {
                    // Sources project horizontally at full strength regardless
                    // of what's below them -- no "capped" requirement.
                    if (newCost < foundCost) { foundCost = newCost; foundType = type; }
                    continue;
                }

                // Only a tile that can't fall any further acts as a
                // horizontal link -- otherwise a waterfall would leak
                // sideways along its entire height instead of staying a
                // clean vertical column, the way it does in vanilla. A
                // falling tile is usually NOT capped (that's what "falling"
                // means -- open space below), except at the very bottom of a
                // waterfall, where it's fed from above but sitting on
                // something solid: that specific tile acts as a full-strength
                // anchor too (this is what makes a pool form beside the base
                // of a waterfall), not just a weaker pass-through link.
                Int32 below = GetWorldBlockOrUnknown(world, nx, y - 1, nz);
                bool belowIsFluid = Fluid::typeOf(below) != Fluid::Type::None;
                bool capped = (below >= 0) && (below != AIR_BLOCK_STATE_ID) && !belowIsFluid;
                if (!capped) continue;

                if (Fluid::isFalling(neighbor)) {
                    if (newCost < foundCost) { foundCost = newCost; foundType = type; }
                    continue;
                }

                if (newCost < foundCost) {
                    if (newCost < 7) queue.push_back({nx, nz, newCost}); // keep searching through it
                }
            }
        }
        if (foundType == Fluid::Type::None) return {Fluid::Type::None, false, 0};
        return {foundType, false, foundCost};
    }

    // restrictType narrows the search to one fluid type (an already-fluid
    // tile only accepts same-type supply, see ResolveFluid); Fluid::Type::None
    // (a fresh air tile) tries both and takes whichever is reachable --
    // stronger one wins if both are. Which type claims a fresh air tile first
    // is still first-come-first-served (an accepted, narrow gap) -- this is
    // unrelated to the water/lava mixing reactions below, which only ever
    // fire on tiles that are ALREADY fluid.
    FluidSupply FindFluidSupply(World& world, int x, int y, int z, Fluid::Type restrictType) {
        if (restrictType != Fluid::Type::None) {
            return FindFluidSupplyForType(world, x, y, z, restrictType);
        }
        FluidSupply water = FindFluidSupplyForType(world, x, y, z, Fluid::Type::Water);
        FluidSupply lava = FindFluidSupplyForType(world, x, y, z, Fluid::Type::Lava);
        if (water.type == Fluid::Type::None) return lava;
        if (lava.type == Fluid::Type::None) return water;
        int waterCost = water.falling ? 0 : water.distance;
        int lavaCost = lava.falling ? 0 : lava.distance;
        return (waterCost <= lavaCost) ? water : lava;
    }

    // Vanilla's water/lava "mixing" rules -- confirmed against minecraft.wiki's
    // Fluid article, Mixing section (cross-checked via two independent
    // fetches, not assumed from memory): a long-stable mechanic, unchanged
    // across versions.
    //   - water touching a lava SOURCE (top or any side, never below)
    //     -> the lava source becomes obsidian
    //   - water touching FLOWING (non-source) lava, any direction except
    //     below -> the flowing lava becomes cobblestone
    //   - ANY lava directly above water -> the water becomes stone instead of
    //     the lava converting. This isn't restricted to this project's
    //     "falling" (level 8) designation -- real vanilla's own fluid tick
    //     re-checks this every time regardless of how the lava got there
    //     (freshly spreading down, or a source/flowing tile just sitting
    //     there), so a static lava tile resting on top of water reacts too,
    //     not only one actively falling into it. Restricting this to
    //     isFalling() only was an earlier, too-narrow reading of the rule
    //     that a real report ("lava on top of flowing water does nothing")
    //     caught.
    // Called from ResolveFluid before normal fluid-supply resolution -- a
    // reaction replaces the tile with a real solid block entirely, so no
    // fluid-supply computation applies to it this tick.
    // The "fizz"/hiss played whenever lava and water react, regardless of
    // which of the three outcomes (obsidian, cobblestone, stone) results --
    // real vanilla plays the same sound for all three. Sound ID (776, for
    // minecraft:block.lava.extinguish) confirmed via this project's own
    // server.jar --reports registries.json ("minecraft:sound_event"), not
    // guessed; category 4 is "block" (docs/network-protocol.md's Stop Sound
    // category table). Volume/pitch are a gameplay-feel approximation, not
    // wire-format, so not decompile-verified the way packet layouts are --
    // matching vanilla's general character (moderate volume, noticeably high
    // pitch) rather than byte-for-byte matching its exact randomization.
    void PlayFluidFizzSound(int x, int y, int z) {
        const Int32 soundId = 776;
        const int category = 4;
        BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, soundId, category](int threshold) {
            return std::make_shared<Sound_Effect_p>(threshold, soundId, category,
                x + 0.5, y + 0.5, z + 0.5, 0.5f, 2.6f, 0LL);
        });
    }

    bool TryApplyFluidReaction(World& world, int x, int y, int z, Int32 current, Fluid::Type currentType) {
        if (currentType == Fluid::Type::Lava) {
            static constexpr int dx[5] = {0, 1, -1, 0, 0};
            static constexpr int dy[5] = {1, 0, 0, 0, 0}; // top + 4 sides, never below
            static constexpr int dz[5] = {0, 0, 0, 1, -1};
            bool touchesWater = false;
            for (int i = 0; i < 5; i++) {
                if (Fluid::typeOf(GetWorldBlockOrUnknown(world, x + dx[i], y + dy[i], z + dz[i])) == Fluid::Type::Water) {
                    touchesWater = true;
                    break;
                }
            }
            if (!touchesWater) return false;

            Int32 result = Fluid::isSource(current) ? OBSIDIAN_BLOCK_STATE_ID : COBBLESTONE_BLOCK_STATE_ID;
            world.setBlock(x, y, z, result);
            BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, result](int threshold) {
                return std::make_shared<Block_Update_p>(threshold, x, y, z, result);
            });
            PlayFluidFizzSound(x, y, z);
            ScheduleFluidNeighbors(x, y, z, 1);
            return true;
        }
        if (currentType == Fluid::Type::Water) {
            Int32 above = GetWorldBlockOrUnknown(world, x, y + 1, z);
            if (Fluid::typeOf(above) == Fluid::Type::Lava) {
                world.setBlock(x, y, z, STONE_BLOCK_STATE_ID);
                BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z](int threshold) {
                    return std::make_shared<Block_Update_p>(threshold, x, y, z, STONE_BLOCK_STATE_ID);
                });
                PlayFluidFizzSound(x, y, z);
                ScheduleFluidNeighbors(x, y, z, 1);
                return true;
            }
        }
        return false;
    }

    // True if any of the 6 face-adjacent neighbors is the opposite fluid
    // type. Used only to decide whether an otherwise-stable tile still needs
    // rechecking every tick (see ResolveFluid's quiesce point below) -- a
    // real, reported bug was that two fluid bodies could sit fully adjacent
    // without ever reacting (or only after an unrelated edit nearby happened
    // to reschedule one of them), since a stable, non-source tile normally
    // stops being scheduled at all once its own supply computation settles.
    // Given the reaction rules above cover every direct-contact direction
    // between water and lava, this always resolves (fires and clears itself)
    // within a bounded number of ticks once true, rather than looping forever.
    bool HasAdjacentOppositeFluid(World& world, int x, int y, int z, Fluid::Type currentType) {
        Fluid::Type opposite = (currentType == Fluid::Type::Water) ? Fluid::Type::Lava : Fluid::Type::Water;
        static constexpr int dx[6] = {1, -1, 0, 0, 0, 0};
        static constexpr int dy[6] = {0, 0, 1, -1, 0, 0};
        static constexpr int dz[6] = {0, 0, 0, 0, 1, -1};
        for (int i = 0; i < 6; i++) {
            if (Fluid::typeOf(GetWorldBlockOrUnknown(world, x + dx[i], y + dy[i], z + dz[i])) == opposite) return true;
        }
        return false;
    }
}

void ResolveFluid(World& world, int x, int y, int z) {
    Int32 current = GetWorldBlockOrUnknown(world, x, y, z);
    if (current < 0) return; // chunk not loaded / out of bounds -- leave it be

    Fluid::Type currentType = Fluid::typeOf(current);
    // Mixing reactions run before anything else -- they apply to source
    // tiles too (rule 1/2 both explicitly cover source lava), so this has to
    // come before the "sources are permanent" shortcut just below.
    if (currentType != Fluid::Type::None && TryApplyFluidReaction(world, x, y, z, current, currentType)) {
        return;
    }
    if (currentType != Fluid::Type::None && Fluid::isSource(current)) {
        // Sources are permanent -- just make sure downstream flow keeps
        // getting re-checked at the right pace.
        ScheduleFluidNeighbors(x, y, z, FluidTickInterval(currentType));
        return;
    }
    // Short grass/poppy/dandelion and crops (wheat/carrots/potatoes/beetroot,
    // any growth stage) are the exception to "fluids only ever flow into open
    // air": real vanilla treats non-solid plants as replaceable by flowing
    // fluid, breaking them with a real drop -- this is the actual mechanic
    // "auto farms" rely on (flood a field of mature crops; the water breaks
    // each one and the current sweeps the drops to a collection point), not
    // a silent removal. Applies for water and lava since both use this
    // project's one shared supply/spread logic below. Everything else solid
    // still fully blocks flow (no waterlogging/passable-block concept
    // otherwise).
    bool currentIsPlant = (current == SHORT_GRASS_STATE_ID || current == POPPY_STATE_ID ||
                            current == DANDELION_STATE_ID || Crop::isCrop(current));
    if (currentType == Fluid::Type::None && current != AIR_BLOCK_STATE_ID && !currentIsPlant) {
        return; // solid block
    }

    // A tile already holding one fluid type is never overridden by a
    // different type reaching it -- only the SAME type can
    // supply/strengthen/sustain an already-fluid position (the opposite type
    // reacts via TryApplyFluidReaction above instead of ever "claiming" the
    // tile outright). A fresh air (or plant) tile has no restriction:
    // whichever type reaches it first claims it (see the note below).
    Fluid::Type restrictType = currentType; // None when currently air or a plant

    FluidSupply supply = FindFluidSupply(world, x, y, z, restrictType);
    // Note: a fresh air tile with both a water and a lava candidate reaching
    // it at the same time picks whichever the search happens to find first
    // (checked in BFS order) -- a narrow, accepted gap, unrelated to the
    // mixing reactions above (which only fire on tiles already holding fluid).

    if (currentIsPlant && supply.type == Fluid::Type::None) {
        // Scheduled (every edit nearby schedules its 6 neighbors) but no
        // fluid actually reaches here -- leave the plant alone. Without this,
        // the "desired == current" quiesce check below would never match
        // (desired defaults to AIR, current is the plant's own block ID) and
        // every plant that ever got scheduled would be destroyed regardless
        // of whether fluid was really present.
        return;
    }

    Int32 desired = (supply.type == Fluid::Type::None) ? AIR_BLOCK_STATE_ID
                  : (supply.falling ? Fluid::fallingId(supply.type) : Fluid::flowingId(supply.type, supply.distance));
    if (currentIsPlant && desired != current) {
        // Real fluid is about to claim this position -- break whatever
        // plant/crop is here first, with the same drops (including mature
        // wheat/beetroot's bonus seeds) a direct player break would give.
        SpawnBlockBreakDrops(x, y, z, floorDiv16(x), floorDiv16(z), current);
    }
    if (desired == current) {
        // Already correct -- normally quiesces here with no further
        // scheduling. But if the opposite fluid is directly adjacent, keep
        // checking every tick instead of going fully quiet: TryApplyFluidReaction
        // already runs unconditionally at the top of this function, so once
        // this fires it'll catch the reaction on the very next call rather
        // than depending entirely on whichever neighbor's last write happened
        // to reschedule this position.
        if (currentType != Fluid::Type::None && HasAdjacentOppositeFluid(world, x, y, z, currentType)) {
            ScheduleFluidCheck(x, y, z, 1);
        }
        return;
    }

    Fluid::Type paceType = (supply.type != Fluid::Type::None) ? supply.type : currentType;
    world.setBlock(x, y, z, desired);
    BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, desired](int threshold) {
        return std::make_shared<Block_Update_p>(threshold, x, y, z, desired);
    });
    ScheduleFluidNeighbors(x, y, z, FluidTickInterval(paceType));
}

namespace {
    // Gameplay-feel tuning, not decompile-verified (like the tree/plant
    // density constants earlier this session) -- real vanilla's actual
    // random-tick-driven pacing/odds are a much more complex function of
    // random tick speed, light, and per-crop growth-speed multipliers this
    // project doesn't model. These constants get the same core loop (grows
    // faster near water + in light, eventually matures) without chasing
    // vanilla's exact timing.
    //
    // A RANGE, not a fixed interval: real vanilla's random tick is a random
    // per-block sample, not a deterministic per-position schedule, so two
    // farmland/crop blocks tilled/planted back-to-back naturally drift apart
    // and never update in lockstep or in the exact order they were placed --
    // a real, reported gap in an earlier fixed-interval version of this
    // (every check landed on the same tick count relative to when it was
    // scheduled, so a batch of farmland all reverted/grew in perfect,
    // visibly-artificial tilling order). A uniform random pick within the
    // range on every single schedule/reschedule call (not just the first)
    // is what actually produces that organic drift -- fixing the RANGE alone
    // without re-rolling each time would've just meant "always +/- the same
    // offset from a fixed base," not real per-block independence.
    constexpr int FARMLAND_CHECK_MIN_INTERVAL = 60;  // 3 seconds at 20 TPS
    constexpr int FARMLAND_CHECK_MAX_INTERVAL = 140; // 7 seconds
    constexpr int CROP_CHECK_MIN_INTERVAL = 60;
    constexpr int CROP_CHECK_MAX_INTERVAL = 140;
    constexpr int CROP_MIN_LIGHT = 8;             // approximate -- real vanilla requires "not dark", exact threshold not decompile-verified here
    constexpr double CROP_GROWTH_CHANCE_WET = 0.15;  // farmland moisture > 0
    constexpr double CROP_GROWTH_CHANCE_DRY = 0.05;  // no wet farmland below (still allowed to grow, just slower)

    double RollUnit() {
        thread_local std::mt19937 generator(std::random_device{}());
        thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator);
    }

    int RandomDelay(int minTicks, int maxTicks) {
        thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(minTicks, maxTicks);
        return distribution(generator);
    }

    // Real vanilla farmland hydration range: a 9x9 column (4 blocks each
    // horizontal direction) spanning 1 block above/below the farmland's own
    // Y, confirmed against minecraft.wiki's Farmland article.
    bool HasNearbyWater(World& world, int x, int y, int z) {
        for (int dx = -4; dx <= 4; dx++) {
            for (int dz = -4; dz <= 4; dz++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (Fluid::typeOf(GetWorldBlockOrUnknown(world, x + dx, y + dy, z + dz)) == Fluid::Type::Water) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    int GetCombinedLight(World& world, int x, int y, int z) {
        int chunkX = floorDiv16(x);
        int chunkZ = floorDiv16(z);
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) return 0;
        int localX = x - chunkX * 16;
        int localZ = z - chunkZ * 16;
        return std::max(chunk->getSkyLight(localX, y, localZ), chunk->getBlockLight(localX, y, localZ));
    }
}

void ResolveCropGrowth(World& world, int x, int y, int z) {
    Int32 current = GetWorldBlockOrUnknown(world, x, y, z);
    if (current < 0) return; // chunk not loaded

    if (Crop::isFarmland(current)) {
        int moisture = Crop::moistureOf(current);
        int newMoisture = HasNearbyWater(world, x, y, z) ? 7 : std::max(0, moisture - 1);
        if (newMoisture != moisture) {
            Int32 newState = Crop::farmlandStateFor(newMoisture);
            world.setBlock(x, y, z, newState);
            BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, newState](int threshold) {
                return std::make_shared<Block_Update_p>(threshold, x, y, z, newState);
            });
        }
        if (newMoisture == 0 && !Crop::isCrop(GetWorldBlockOrUnknown(world, x, y + 1, z))) {
            // Real vanilla also reverts farmland via player/mob trampling --
            // not modeled (no server-side jump/fall detection exists here,
            // movement is client-authoritative), only the moisture-based
            // revert is implemented.
            world.setBlock(x, y, z, DIRT_BLOCK_STATE_ID);
            BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z](int threshold) {
                return std::make_shared<Block_Update_p>(threshold, x, y, z, DIRT_BLOCK_STATE_ID);
            });
            return; // it's dirt now -- no more farmland checks needed
        }
        CropGrowthQueue::getInstance().schedule(x, y, z, RandomDelay(FARMLAND_CHECK_MIN_INTERVAL, FARMLAND_CHECK_MAX_INTERVAL));
        return;
    }

    if (Crop::isCrop(current)) {
        Crop::Type type = Crop::typeOf(current);
        int age = Crop::ageOf(current);
        int maxAge = Crop::maxAge(type);
        if (age >= maxAge) return; // fully grown -- nothing left to schedule, matches fluid sources' permanent early-exit

        if (GetCombinedLight(world, x, y, z) >= CROP_MIN_LIGHT) {
            Int32 below = GetWorldBlockOrUnknown(world, x, y - 1, z);
            bool wet = Crop::isFarmland(below) && Crop::moistureOf(below) > 0;
            if (RollUnit() < (wet ? CROP_GROWTH_CHANCE_WET : CROP_GROWTH_CHANCE_DRY)) {
                Int32 newState = Crop::stateFor(type, age + 1);
                world.setBlock(x, y, z, newState);
                BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, newState](int threshold) {
                    return std::make_shared<Block_Update_p>(threshold, x, y, z, newState);
                });
            }
        }
        CropGrowthQueue::getInstance().schedule(x, y, z, RandomDelay(CROP_CHECK_MIN_INTERVAL, CROP_CHECK_MAX_INTERVAL));
    }
}

void TryPickupNearbyItems(PacketContext& cont, int threshold, Player& player) {
    // A single spherical radius around player.getY() (the FEET position)
    // was wrong: real vanilla pickup is really "does the item overlap the
    // player's whole body volume" -- a 0.6-wide, ~1.8-tall box from the
    // player's feet up to over their head, not a 1-block sphere centered on
    // the feet. Horizontal reach and vertical reach need separate checks, or
    // any item sitting near head height (dy approaching 1.8 on its own)
    // fails the combined dx^2+dy^2+dz^2 radius before dx/dz are even
    // considered -- a real, reported bug (items adjacent to the player's
    // head were never pickup-able, only ones down near their feet).
    const double PICKUP_HORIZONTAL_RADIUS_SQUARED = 1.0; // ~1 block, approximates vanilla's horizontal reach
    const double PLAYER_HEIGHT = 1.8; // real vanilla standing hitbox height
    const double PICKUP_VERTICAL_MARGIN = 0.5; // slop above/below the hitbox itself, matching vanilla's own inflate
    const double MIN_PICKUP_AGE_SECONDS = 0.5; // matches vanilla's 10-tick pickup delay

    ItemEntityManager& manager = ItemEntityManager::getInstance();
    auto now = std::chrono::steady_clock::now();
    for (const ItemEntity& entity : manager.snapshot()) {
        double age = std::chrono::duration<double>(now - entity.spawnTime).count();
        if (age < MIN_PICKUP_AGE_SECONDS) continue;

        double dx = entity.x - player.getX();
        double dz = entity.z - player.getZ();
        if (dx * dx + dz * dz > PICKUP_HORIZONTAL_RADIUS_SQUARED) continue;
        if (entity.y < player.getY() - PICKUP_VERTICAL_MARGIN ||
            entity.y > player.getY() + PLAYER_HEIGHT + PICKUP_VERTICAL_MARGIN) continue;

        if (!player.hasRoomFor(entity.itemId)) continue;
        if (!manager.tryClaim(entity.entityId)) continue; // someone else got it first

        std::vector<int> changedSlots;
        player.addItemToHotbar(entity.itemId, entity.count, changedSlots); // guaranteed to fit -- hasRoomFor already checked

        int entityId = entity.entityId;
        int collectorId = player.getEntityId();
        int count = entity.count;
        BroadcastToChunkViewers(entity.chunkX, entity.chunkZ, [entityId, collectorId, count](int broadcastThreshold) {
            return std::make_shared<Pickup_Item_p>(broadcastThreshold, entityId, collectorId, count);
        });
        BroadcastToChunkViewers(entity.chunkX, entity.chunkZ, [entityId](int broadcastThreshold) {
            return std::make_shared<Remove_Entities_p>(broadcastThreshold, entityId);
        });

        for (int slot : changedSlots) {
            int containerSlot = 36 + slot; // player inventory: hotbar occupies slots 36-44
            InventorySlot updated = player.getHotbar()[slot]; // copy -- getHotbar() returns by value now
            cont.connection.addPacket(std::make_shared<Set_Container_Slot_p>(threshold, 0, player.advanceContainerStateId(), containerSlot, updated.itemId, updated.count));
        }
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
            world.chunkViewerRemoved(x, z);
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
    bool onGround = deserializeBool(in_buff);

    Player& player = cont.connection.getPlayer();
    double oldX = player.getX(), oldY = player.getY(), oldZ = player.getZ();
    player.setPosition(x, y, z);

    // Server has no swim/buoyancy physics of its own -- a real client already
    // slows itself down locally once it sees a real water block (stage 1).
    // This only tracks the flag so OTHER clients see this player as swimming
    // (Player::getPose(), broadcast via Set_Player_Pose_Metadata_p).
    bool nowInWater = Fluid::typeOf(GetWorldBlockOrUnknown(World::getInstance(),
        static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(z)))) == Fluid::Type::Water;
    if (nowInWater != player.isInWater()) {
        player.setInWater(nowInWater);
        PlayerVisibilityManager::getInstance().broadcastPoseChange(cont.connection.shared_from_this());
    }

    int threshold = cont.connection.getCompressionThreshold();
    TryPickupNearbyItems(cont, threshold, player);
    PlayerVisibilityManager::getInstance().broadcastMovement(cont.connection.shared_from_this(),
        oldX, oldY, oldZ, true, false, onGround);
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
    bool onGround = deserializeBool(in_buff);

    Player& player = cont.connection.getPlayer();
    double oldX = player.getX(), oldY = player.getY(), oldZ = player.getZ();
    player.setPosition(x, y, z);
    player.setRotation(yaw, pitch);

    // See Set_Player_Position_p::deserialize for why this only tracks a flag
    // (no server-side swim physics -- the real client handles that locally).
    bool nowInWater = Fluid::typeOf(GetWorldBlockOrUnknown(World::getInstance(),
        static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(z)))) == Fluid::Type::Water;
    if (nowInWater != player.isInWater()) {
        player.setInWater(nowInWater);
        PlayerVisibilityManager::getInstance().broadcastPoseChange(cont.connection.shared_from_this());
    }

    int threshold = cont.connection.getCompressionThreshold();
    TryPickupNearbyItems(cont, threshold, player);
    PlayerVisibilityManager::getInstance().broadcastMovement(cont.connection.shared_from_this(),
        oldX, oldY, oldZ, true, true, onGround);
    int newCenterX = static_cast<int>(std::floor(x / 16.0));
    int newCenterZ = static_cast<int>(std::floor(z / 16.0));
    UpdateLoadedChunks(cont, threshold, player, newCenterX, newCenterZ);
}

void Set_Player_Rotation_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Player_Rotation_p::deserialize(): Received.");
    #endif
    float yaw = deserializeFloat(in_buff);
    float pitch = deserializeFloat(in_buff);
    bool onGround = deserializeBool(in_buff);

    // Sent when the client turns without moving its feet, as opposed to
    // Set_Player_Position_and_Rotation_p which only fires alongside a position
    // change -- without this, Player's stored yaw/pitch goes stale the moment
    // someone stands still and just looks around (e.g. aiming a Q-drop).
    Player& player = cont.connection.getPlayer();
    double x = player.getX(), y = player.getY(), z = player.getZ();
    player.setRotation(yaw, pitch);
    PlayerVisibilityManager::getInstance().broadcastMovement(cont.connection.shared_from_this(),
        x, y, z, false, true, onGround);
}

void Serverbound_Keep_Alive_play_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Serverbound_Keep_Alive_play_p::deserialize(): Received.");
    #endif
    // TODO: validate against the ID we last sent once Connection proactively
    // sends Keep Alives on a timer; nothing does yet, so there's nothing to check.
}

namespace {
    // *** CHECKLIST: read this before adding any new block to BlockIds.hpp or
    // BlockTable.cpp. ***
    //
    // Real vanilla clients never send a "Finished digging" (status 2) follow
    // up for a block with 0 hardness ("instamined") -- only "Started
    // digging" (status 0), same as they do for Creative mode (see
    // docs/network-protocol.md's Player Action table). A block missing from
    // the list below will still visually appear to break for the player
    // (the client predicts the removal locally regardless), but the SERVER
    // will silently never remove it or drop anything, since it's waiting on
    // a status 2 that will never arrive -- exactly the bug this function was
    // added to fix (see docs/internal-documentation.md, "trees and plants").
    //
    // How to check whether a new block needs to be added here: run
    // `server.jar --reports` and look up the block in the generated
    // blocks.json -- its `definition.type` field reflects vanilla's real
    // block-behavior category. Every block whose category is one of these
    // is 0-hardness in real vanilla (confirmed for the three listed below;
    // re-verify a new category against a fresh report rather than assuming
    // this list is exhaustive): minecraft:flower, minecraft:tall_grass,
    // minecraft:double_plant, minecraft:sapling, minecraft:torch,
    // minecraft:redstone_wire, minecraft:crop, minecraft:snow_layer,
    // minecraft:fire. This project has no general block-hardness system (a
    // deliberate scope decision) and doesn't load blocks.json at runtime at
    // all, so this stays a small, explicit, hand-maintained list rather than
    // a data-driven lookup -- same convention BlockTable.cpp itself already
    // uses for other real, hand-transcribed vanilla facts.
    //
    // *** ADD NEW ENTRIES HERE, ONE PER LINE. *** Nowhere else needs to change.
    constexpr Int32 INSTANT_BREAK_BLOCKS[] = {
        SHORT_GRASS_STATE_ID,
        POPPY_STATE_ID,
        DANDELION_STATE_ID,
    };

    bool IsInstantBreakBlock(Int32 blockStateId) {
        for (Int32 id : INSTANT_BREAK_BLOCKS) {
            if (id == blockStateId) return true;
        }
        // Crops (wheat/carrots/potatoes/beetroots, any growth stage) are
        // real vanilla:crop/carrot/potato/beetroot definition.type blocks --
        // confirmed 0-hardness, same category this whole list exists for.
        // Checked via Crop::isCrop() rather than 28 individual entries (one
        // per age state across 4 crop types) since that helper already
        // exists and covers exactly this set -- missed when farming was
        // first added, exactly the gap this checklist warns about.
        if (Crop::isCrop(blockStateId)) return true;
        return false;
    }
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
    // minecraft:item entity-type registry ID, sourced from the vanilla data
    // generator report, not guessed. Shared by both the break-drop and
    // Q-drop paths below.
    const int ITEM_ENTITY_TYPE_ID = 58;

    // No server-side mining-time/hardness validation (see docs/general-documentation.md,
    // "Accepted gaps") -- trust status 2 (Finished digging) in any gamemode.
    // Status 0 (Started digging) alone is also enough for Creative -- and,
    // per docs/network-protocol.md's Player Action table: "If the block was
    // instamined or the player is in creative mode, the client will NOT send
    // Status = Finished digging, and will assume the server completed the
    // destruction." This project has no general block-hardness system, but
    // real vanilla flowers/grass tufts have 0 hardness (instamined) in EVERY
    // gamemode, not just Creative -- a real, confirmed bug: breaking one of
    // these in Survival looked like it worked (the client predicts the
    // removal locally and never sends a follow-up status 2, exactly as
    // documented), but the server was only watching for status 2 outside
    // Creative, so it never actually removed the block or dropped anything.
    // The block reappearing later was the client's unconfirmed local
    // prediction eventually reconciling back to the server's real,
    // never-actually-changed state.
    if (status == 0 || status == 2) {
        World& world = World::getInstance();
        int chunkX = floorDiv16(loc.x);
        int chunkZ = floorDiv16(loc.z);

        int threshold = cont.connection.getCompressionThreshold();

        // Read the block being targeted before it's gone, both to resolve
        // its drop and to check whether it's a known-instant block (see
        // above). If the chunk isn't actually cached (shouldn't happen for a
        // chunk the player is standing in and can see, but was previously
        // silently treated as "the block was air" -- which meant a break
        // here still acked/broadcast Block_Update_p(AIR) to the client,
        // making it LOOK removed, while the real server-side block was
        // untouched). Now this bails out cleanly instead of lying to the client.
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) {
            Console::getConsole().Error("Player_Action_p::deserialize(): break at (" + std::to_string(loc.x) + "," +
                std::to_string(loc.y) + "," + std::to_string(loc.z) + ") targeted chunk (" + std::to_string(chunkX) +
                "," + std::to_string(chunkZ) + "), which isn't cached -- ignoring the break.");
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            return;
        }
        Int32 previousBlock = chunk->getBlock(loc.x - chunkX * 16, loc.y, loc.z - chunkZ * 16);

        bool shouldBreak = (status == 2) || (status == 0 && (player.getGamemode() == CREATIVE_GAMEMODE || IsInstantBreakBlock(previousBlock)));
        if (!shouldBreak) return; // "Started digging" on an ordinary (non-instant) block in Survival -- the real break arrives later as status 2

        if (Fluid::typeOf(previousBlock) != Fluid::Type::None) {
            // Fluids aren't "mined" by punching in vanilla -- picking one up
            // requires an empty bucket (a future interactions-stage feature).
            // Still ack, since the client already predicted the break locally.
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            return;
        }

        if (!world.setBlock(loc.x, loc.y, loc.z, AIR_BLOCK_STATE_ID)) {
            // Same "don't lie to the client" reasoning as above -- the chunk
            // was cached a moment ago (just read successfully) but isn't
            // anymore, an actual race with eviction/unloading. Rare, but
            // possible, and worth knowing about if it ever fires.
            Console::getConsole().Error("Player_Action_p::deserialize(): world.setBlock failed for a break at (" +
                std::to_string(loc.x) + "," + std::to_string(loc.y) + "," + std::to_string(loc.z) +
                ") -- the chunk was evicted between the read and the write.");
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            return;
        }
        BroadcastToChunkViewers(chunkX, chunkZ, [loc](int broadcastThreshold) {
            return std::make_shared<Block_Update_p>(broadcastThreshold, loc.x, loc.y, loc.z, AIR_BLOCK_STATE_ID);
        });
        cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));

        // The block just vacated might have been the only thing holding up a
        // sand/gravel block directly above it...
        CheckGravityBlock(world, loc.x, loc.y + 1, loc.z);
        // ...or a plant that needs solid ground under it.
        CheckPlantSupport(world, loc.x, loc.y + 1, loc.z);
        // ...or the only thing blocking a neighboring fluid from spreading
        // into the new opening.
        ScheduleFluidNeighbors(loc.x, loc.y, loc.z, 1);

        // Tracked server-side by ItemEntityManager so it can later be picked
        // up (TryPickupNearbyItems) or despawned (ItemDespawnSystem) -- the
        // Spawn_Entity_p/Set_Entity_Metadata_p broadcast below only tells
        // clients what to render.
        // Creative instantly removes the block with no drop, matching vanilla --
        // only Survival/Adventure actually spawn a pickup-able item entity.
        Int32 dropItemId = -1;
        Int32 dropCount = 1;
        Int32 bonusDropItemId = -1;
        Int32 bonusDropCount = 0;
        if (player.getGamemode() != CREATIVE_GAMEMODE) {
            // ToolInfo is always default (no Fortune, no Silk Touch) for now --
            // this project can't yet receive/store enchantment data on a held
            // item at all (see BlockDropTable::ToolInfo's own comment). Once
            // that groundwork exists, this should read the player's actual
            // held item's enchantments instead of default-constructing.
            BlockDropTable::ToolInfo tool;
            Int32 overrideItemId, overrideCount;
            // BlockDropTable covers blocks whose drop isn't just themselves
            // (stone -> cobblestone, oak_leaves -> a chance of oak_sapling,
            // etc.) -- only fall back to the default direct block<->item
            // mapping (drop itself, count 1) when this block has no override entry.
            if (BlockDropTable::CheckDrop(previousBlock, tool, overrideItemId, overrideCount)) {
                dropItemId = overrideItemId;
                dropCount = overrideCount;
            } else {
                dropItemId = blockStateIdToItemId(previousBlock);
            }
            // A handful of blocks (mature wheat/beetroot) drop a second,
            // independent item stack on top of the above -- see
            // BlockDropTable::CheckBonusDrop's own comment.
            BlockDropTable::CheckBonusDrop(previousBlock, tool, bonusDropItemId, bonusDropCount);
        }
        if (dropItemId >= 0 && dropCount > 0) {
            double dropX = loc.x + 0.5, dropY = loc.y + 0.5, dropZ = loc.z + 0.5;
            ItemEntity dropped = ItemEntityManager::getInstance().spawn(dropItemId, dropCount, dropX, dropY, dropZ, chunkX, chunkZ);
            std::vector<long> uuid = generateRandomUUID(); // one-time, not persisted -- only needed for this Spawn_Entity_p
            int entityId = dropped.entityId;
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, dropX, dropY, dropZ, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
                return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, dropX, dropY, dropZ);
            });
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, dropItemId, dropCount](int broadcastThreshold) {
                return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, dropItemId, dropCount);
            });
        }
        if (bonusDropItemId >= 0 && bonusDropCount > 0) {
            double dropX = loc.x + 0.5, dropY = loc.y + 0.5, dropZ = loc.z + 0.5;
            ItemEntity bonusDropped = ItemEntityManager::getInstance().spawn(bonusDropItemId, bonusDropCount, dropX, dropY, dropZ, chunkX, chunkZ);
            std::vector<long> bonusUuid = generateRandomUUID();
            int bonusEntityId = bonusDropped.entityId;
            BroadcastToChunkViewers(chunkX, chunkZ, [bonusEntityId, bonusUuid, dropX, dropY, dropZ, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
                return std::make_shared<Spawn_Entity_p>(broadcastThreshold, bonusEntityId, bonusUuid, ITEM_ENTITY_TYPE_ID, dropX, dropY, dropZ);
            });
            BroadcastToChunkViewers(chunkX, chunkZ, [bonusEntityId, bonusDropItemId, bonusDropCount](int broadcastThreshold) {
                return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, bonusEntityId, bonusDropItemId, bonusDropCount);
            });
        }
    } else if (status == 3 || status == 4) { // Drop item stack / Drop item (the Q key)
        int selectedSlot = player.getSelectedSlot();
        InventorySlot held = player.getHotbar()[selectedSlot]; // copy: setHotbarSlot below must not alias this read
        if (held.itemId >= 0 && held.count > 0) {
            Int32 dropCount = (status == 3) ? held.count : 1;
            Int32 newCount = held.count - dropCount;
            Int32 newItemId = (newCount > 0) ? held.itemId : -1;
            player.setHotbarSlot(selectedSlot, newItemId, newCount);
            int threshold = cont.connection.getCompressionThreshold();
            int containerSlot = 36 + selectedSlot; // player inventory: hotbar occupies slots 36-44
            cont.connection.addPacket(std::make_shared<Set_Container_Slot_p>(threshold, 0, player.advanceContainerStateId(), containerSlot, newItemId, newCount));

            // Standard forward-facing direction vector from yaw/pitch, tossed
            // with a small upward kick -- an approximation of vanilla's drop
            // arc, not a wire-format detail, so no decompile verification needed.
            const double PI = 3.14159265358979323846;
            const double TOSS_SPEED = 0.3;
            double yawRad = player.getYaw() * PI / 180.0;
            double pitchRad = player.getPitch() * PI / 180.0;
            double dirX = -std::sin(yawRad) * std::cos(pitchRad);
            double dirY = -std::sin(pitchRad);
            double dirZ = std::cos(yawRad) * std::cos(pitchRad);
            double vx = dirX * TOSS_SPEED;
            double vy = dirY * TOSS_SPEED + 0.1;
            double vz = dirZ * TOSS_SPEED;

            double dropX = player.getX(), dropY = player.getY() + 1.2, dropZ = player.getZ();
            int chunkX = floorDiv16(static_cast<int>(std::floor(dropX)));
            int chunkZ = floorDiv16(static_cast<int>(std::floor(dropZ)));
            ItemEntity dropped = ItemEntityManager::getInstance().spawn(held.itemId, dropCount, dropX, dropY, dropZ, chunkX, chunkZ, vx, vy, vz);
            std::vector<long> uuid = generateRandomUUID();
            int entityId = dropped.entityId;
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, dropX, dropY, dropZ, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
                return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, dropX, dropY, dropZ);
            });
            Int32 metaItemId = held.itemId;
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, metaItemId, dropCount](int broadcastThreshold) {
                return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, metaItemId, dropCount);
            });
            // Spawn_Entity_p always encodes zero velocity -- follow up immediately
            // so the client's own local physics picks up the toss instead of
            // rendering the item as motionless until ItemPhysicsSystem's next
            // meaningful correction.
            BroadcastToChunkViewers(chunkX, chunkZ, [entityId, vx, vy, vz](int broadcastThreshold) {
                return std::make_shared<Set_Entity_Velocity_p>(broadcastThreshold, entityId, vx, vy, vz);
            });
        }
    }
}

void Player_Command_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Player_Command_p::deserialize(): Received.");
    #endif
    deserializeVarInt(in_buff); // Entity ID: always the sender's own, unused.
    int actionId = deserializeVarInt(in_buff);
    deserializeVarInt(in_buff); // Jump Boost: horse-related, unused.

    Player& player = cont.connection.getPlayer();
    bool changed = true;
    switch (actionId) {
        case 0: player.setSneaking(true); break;
        case 1: player.setSneaking(false); break;
        case 3: player.setSprinting(true); break;
        case 4: player.setSprinting(false); break;
        default: changed = false; break; // leave bed / horse jump / vehicle inventory / elytra: not implemented
    }

    if (changed) {
        PlayerVisibilityManager::getInstance().broadcastPoseChange(cont.connection.shared_from_this());
    }
}

void Chat_Message_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Chat_Message_p::deserialize(): Received.");
    #endif
    string message = deserializeString(in_buff);
    deserializeLong(in_buff); // Timestamp: unused, no chat signing.
    deserializeLong(in_buff); // Salt: unused, no chat signing.
    bool hasSignature = deserializeBool(in_buff);
    if (hasSignature) {
        in_buff.erase(in_buff.begin(), in_buff.begin() + 256); // Signature: never verified.
    }
    deserializeVarInt(in_buff); // Message Count: unused, no signature-chain tracking.
    in_buff.erase(in_buff.begin(), in_buff.begin() + 3); // Acknowledged (Fixed BitSet(20) = 3 bytes): unused.

    Player& player = cont.connection.getPlayer();
    Console::getConsole().Entry("<" + player.getUsername() + "> " + message);
    BroadcastDisguisedChat(player.getUsername(), message, "minecraft:chat");
}

void Chat_Command_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Chat_Command_p::deserialize(): Received.");
    #endif
    string commandLine = deserializeString(in_buff);
    if (!commandLine.empty() && commandLine[0] == '/') {
        commandLine.erase(commandLine.begin());
    }

    std::istringstream iss(commandLine);
    string name;
    iss >> name;
    std::vector<string> args;
    string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    PlayerCommandSender sender(cont.connection.shared_from_this());
    CommandRegistry::getInstance().dispatch(sender, name, args);
}

namespace {
    const int CREATIVE_GAMEMODE = 1;

    // Survival/Adventure consume one from the held stack; Creative's
    // infinite-item slot is left untouched, matching vanilla. Shared by the
    // generic placement path below and the till/plant/bone-meal handlers --
    // all four need the exact same "decrement, empty at 0, echo back" logic.
    void ConsumeOneFromHeldSlot(PacketContext& cont, int threshold, Player& player, int selectedSlot, const InventorySlot& held) {
        if (player.getGamemode() == CREATIVE_GAMEMODE) return;
        Int32 newCount = held.count - 1;
        Int32 newItemId = (newCount > 0) ? held.itemId : -1;
        if (newCount <= 0) newCount = 0;
        player.setHotbarSlot(selectedSlot, newItemId, newCount);
        int containerSlot = 36 + selectedSlot; // player inventory: hotbar occupies slots 36-44
        cont.connection.addPacket(std::make_shared<Set_Container_Slot_p>(threshold, 0, player.advanceContainerStateId(), containerSlot, newItemId, newCount));
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
    int selectedSlot = player.getSelectedSlot();
    InventorySlot held = player.getHotbar()[selectedSlot]; // copy: mutating the slot below must not alias this read
    World& world = World::getInstance();

    // Farming: till / plant / bone meal, checked before the generic
    // placement path below (a hoe/seed/bone-meal item would otherwise either
    // no-op via itemIdToBlockStateId returning -1, or -- for seeds, which
    // also have a placeable block form -- place the wrong thing). Real item/
    // block names and IDs sourced from a fresh server.jar --reports run, not
    // guessed.
    {
        const Int32 BONE_MEAL_ITEM_ID = 960;
        const Int32 WHEAT_SEEDS_ITEM_ID = 853;
        const Int32 CARROT_ITEM_ID = 1097;
        const Int32 POTATO_ITEM_ID = 1098;
        const Int32 BEETROOT_SEEDS_ITEM_ID = 1155;

        Int32 clickedBlock = GetWorldBlockOrUnknown(world, loc.x, loc.y, loc.z);
        Int32 aboveClicked = GetWorldBlockOrUnknown(world, loc.x, loc.y + 1, loc.z);

        // Till: hoe on dirt/grass_block with air directly above -> farmland
        // (moisture 0). No durability cost -- this project has no durability
        // system for any tool yet.
        if (ItemProperties::getItemCategory(held.itemId) == ItemCategory::Hoe
            && (clickedBlock == DIRT_BLOCK_STATE_ID || clickedBlock == GRASS_BLOCK_STATE_ID)
            && aboveClicked == AIR_BLOCK_STATE_ID) {
            Int32 x = loc.x, y = loc.y, z = loc.z;
            Int32 farmlandId = Crop::farmlandStateFor(0);
            world.setBlock(x, y, z, farmlandId);
            BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, farmlandId](int broadcastThreshold) {
                return std::make_shared<Block_Update_p>(broadcastThreshold, x, y, z, farmlandId);
            });
            CropGrowthQueue::getInstance().schedule(x, y, z, RandomDelay(FARMLAND_CHECK_MIN_INTERVAL, FARMLAND_CHECK_MAX_INTERVAL));
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            return;
        }

        // Plant: a seed item on farmland with air directly above -> that
        // crop's age-0 state, planted in the position ABOVE the farmland
        // (always straight up, matching real vanilla -- not face-relative
        // like a normal block placement).
        Crop::Type seedType = Crop::Type::None;
        if (held.itemId == WHEAT_SEEDS_ITEM_ID) seedType = Crop::Type::Wheat;
        else if (held.itemId == CARROT_ITEM_ID) seedType = Crop::Type::Carrots;
        else if (held.itemId == POTATO_ITEM_ID) seedType = Crop::Type::Potatoes;
        else if (held.itemId == BEETROOT_SEEDS_ITEM_ID) seedType = Crop::Type::Beetroot;
        if (seedType != Crop::Type::None && Crop::isFarmland(clickedBlock) && aboveClicked == AIR_BLOCK_STATE_ID && held.count > 0) {
            Int32 x = loc.x, y = loc.y + 1, z = loc.z;
            Int32 cropId = Crop::stateFor(seedType, 0);
            world.setBlock(x, y, z, cropId);
            BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, cropId](int broadcastThreshold) {
                return std::make_shared<Block_Update_p>(broadcastThreshold, x, y, z, cropId);
            });
            CropGrowthQueue::getInstance().schedule(x, y, z, RandomDelay(CROP_CHECK_MIN_INTERVAL, CROP_CHECK_MAX_INTERVAL));
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            ConsumeOneFromHeldSlot(cont, threshold, player, selectedSlot, held);
            return;
        }

        // Bone meal: instantly jumps a crop's age (capped at its real max).
        // Jump size is a gameplay-feel approximation (1-3), not vanilla-exact
        // -- same treatment as the growth-chance constants near
        // ResolveCropGrowth. Real vanilla also works on saplings/grass;
        // crops only for this pass.
        if (held.itemId == BONE_MEAL_ITEM_ID && Crop::isCrop(clickedBlock) && held.count > 0) {
            Crop::Type cropType = Crop::typeOf(clickedBlock);
            int age = Crop::ageOf(clickedBlock);
            int maxAge = Crop::maxAge(cropType);
            if (age < maxAge) {
                thread_local std::mt19937 boneMealGenerator(std::random_device{}());
                std::uniform_int_distribution<int> jumpDist(1, 3);
                int newAge = std::min(maxAge, age + jumpDist(boneMealGenerator));
                Int32 x = loc.x, y = loc.y, z = loc.z;
                Int32 newState = Crop::stateFor(cropType, newAge);
                world.setBlock(x, y, z, newState);
                BroadcastToChunkViewers(floorDiv16(x), floorDiv16(z), [x, y, z, newState](int broadcastThreshold) {
                    return std::make_shared<Block_Update_p>(broadcastThreshold, x, y, z, newState);
                });
            }
            cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));
            ConsumeOneFromHeldSlot(cont, threshold, player, selectedSlot, held);
            return;
        }
    }

    // Water/lava buckets aren't in BlockTable (they have no real placeable
    // "block" item -- vanilla's bucket-swap-to-empty mechanic is a future
    // interactions-stage feature), so they're special-cased directly to a
    // fluid source rather than going through itemIdToBlockStateId. Creative
    // only for now: in Survival the bucket would never empty, which would be
    // more confusing than just not supporting it yet.
    const int WATER_BUCKET_ITEM_ID = 909;
    const int LAVA_BUCKET_ITEM_ID = 910;
    Int32 blockStateId;
    if (player.getGamemode() == CREATIVE_GAMEMODE && held.itemId == WATER_BUCKET_ITEM_ID) {
        blockStateId = Fluid::sourceId(Fluid::Type::Water);
    } else if (player.getGamemode() == CREATIVE_GAMEMODE && held.itemId == LAVA_BUCKET_ITEM_ID) {
        blockStateId = Fluid::sourceId(Fluid::Type::Lava);
    } else {
        blockStateId = itemIdToBlockStateId(held.itemId);
    }
    if (blockStateId < 0 || held.count <= 0) {
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

    world.setBlock(px, py, pz, blockStateId);
    int chunkX = floorDiv16(px);
    int chunkZ = floorDiv16(pz);
    BroadcastToChunkViewers(chunkX, chunkZ, [px, py, pz, blockStateId](int broadcastThreshold) {
        return std::make_shared<Block_Update_p>(broadcastThreshold, px, py, pz, blockStateId);
    });
    cont.connection.addPacket(std::make_shared<Acknowledge_Block_Change_p>(threshold, sequence));

    // A sand/gravel block placed directly onto open air starts falling
    // immediately, matching vanilla.
    CheckGravityBlock(world, px, py, pz);
    // A placed fluid source needs its neighbors to notice it's there and
    // start spreading; a placed solid block may have just cut off an
    // existing fluid's supply.
    ScheduleFluidNeighbors(px, py, pz, 1);

    ConsumeOneFromHeldSlot(cont, threshold, player, selectedSlot, held);
}

// Sent by the client while the Creative inventory screen is open: picking an
// item from the creative tabs and placing it in a slot, picking an item back
// up (empties the slot), or dragging it out of the window entirely (Slot -1,
// spawns a dropped item -- see the packet's own documented semantics in
// docs/network-protocol.md).
void Set_Creative_Mode_Slot_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Creative_Mode_Slot_p::deserialize(): Received.");
    #endif
    Int16 slot = deserializeShort(in_buff);
    DecodedSlot item = unpackSlot(in_buff);

    Player& player = cont.connection.getPlayer();
    // Only ever legitimately sent while the (Creative-only) inventory screen
    // is open -- a non-Creative sender is stale or an attempted free-item
    // exploit, so this is rejected outright rather than honored.
    if (player.getGamemode() != CREATIVE_GAMEMODE) return;

    if (slot == -1) {
        // Dropped outside the window -- spawn it as a real dropped item,
        // reusing the same entity/physics infrastructure the Q-drop path
        // (Player_Action_p status 3/4) already uses.
        if (!item.present) return;
        double px = player.getX(), py = player.getY(), pz = player.getZ();
        int chunkX = floorDiv16(static_cast<int>(std::floor(px)));
        int chunkZ = floorDiv16(static_cast<int>(std::floor(pz)));
        ItemEntity dropped = ItemEntityManager::getInstance().spawn(item.itemId, item.count, px, py + 1.0, pz, chunkX, chunkZ);
        std::vector<long> uuid = generateRandomUUID();
        int entityId = dropped.entityId;
        const int ITEM_ENTITY_TYPE_ID = 58;
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, px, py, pz, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
            return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, px, py + 1.0, pz);
        });
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, item](int broadcastThreshold) {
            return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, item.itemId, item.count);
        });
        return;
    }

    if (slot < 0 || slot >= Player::TOTAL_SLOTS) return;
    player.setSlot(slot, item.present ? item.itemId : -1, item.present ? item.count : 0);
    int threshold = cont.connection.getCompressionThreshold();
    cont.connection.addPacket(std::make_shared<Set_Container_Slot_p>(threshold, 0, player.advanceContainerStateId(), slot, item.present ? item.itemId : -1, item.present ? item.count : 0));
}

namespace {
    // Spawns a real, pickup-able dropped item at the player's own position --
    // the same entity/physics infrastructure Player_Action_p's Q-drop and
    // Set_Creative_Mode_Slot_p's slot=-1 drop already use (kept separate
    // there rather than migrated, to avoid touching already-working code) --
    // extracted here since Click_Container_p needs the identical sequence in
    // two more places (outside-window click, drop-key mode).
    void SpawnDroppedItemEntity(Player& player, Int32 itemId, Int32 count) {
        if (itemId < 0 || count <= 0) return;
        double px = player.getX(), py = player.getY(), pz = player.getZ();
        int chunkX = floorDiv16(static_cast<int>(std::floor(px)));
        int chunkZ = floorDiv16(static_cast<int>(std::floor(pz)));
        ItemEntity dropped = ItemEntityManager::getInstance().spawn(itemId, count, px, py + 1.0, pz, chunkX, chunkZ);
        std::vector<long> uuid = generateRandomUUID();
        int entityId = dropped.entityId;
        const int ITEM_ENTITY_TYPE_ID = 58;
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, uuid, px, py, pz, ITEM_ENTITY_TYPE_ID](int broadcastThreshold) {
            return std::make_shared<Spawn_Entity_p>(broadcastThreshold, entityId, uuid, ITEM_ENTITY_TYPE_ID, px, py + 1.0, pz);
        });
        BroadcastToChunkViewers(chunkX, chunkZ, [entityId, itemId, count](int broadcastThreshold) {
            return std::make_shared<Set_Entity_Metadata_p>(broadcastThreshold, entityId, itemId, count);
        });
    }

    // Mode 0: normal left/right click. Slot == -999 means "outside the
    // window" (drop from cursor); otherwise picks up/places/merges/swaps
    // between the cursor and the clicked slot. See docs/network-protocol.md's
    // Click Container section for the exact button semantics this mirrors.
    void HandleNormalClick(Player& player, int slot, int button) {
        InventorySlot cursor = player.getCarriedItem();
        if (slot == -999) {
            if (cursor.itemId < 0 || cursor.count <= 0) return;
            if (button == 0) {
                SpawnDroppedItemEntity(player, cursor.itemId, cursor.count);
                player.setCarriedItem(-1, 0);
            } else {
                SpawnDroppedItemEntity(player, cursor.itemId, 1);
                Int32 newCount = cursor.count - 1;
                player.setCarriedItem(newCount > 0 ? cursor.itemId : -1, newCount);
            }
            return;
        }
        if (slot < 0 || slot >= Player::TOTAL_SLOTS) return;
        InventorySlot target = player.getSlot(slot);

        if (cursor.itemId < 0) {
            if (target.itemId < 0) return; // both empty
            if (button == 0) {
                player.setCarriedItem(target.itemId, target.count);
                player.setSlot(slot, -1, 0);
            } else {
                // Right click on a stack: half picked up, half (the smaller
                // amount if odd) left behind -- per docs/network-protocol.md.
                Int32 stays = target.count / 2;
                Int32 pickedUp = target.count - stays;
                player.setCarriedItem(target.itemId, pickedUp);
                player.setSlot(slot, stays > 0 ? target.itemId : -1, stays);
            }
            return;
        }

        if (target.itemId < 0) {
            if (button == 0) {
                player.setSlot(slot, cursor.itemId, cursor.count);
                player.setCarriedItem(-1, 0);
            } else {
                player.setSlot(slot, cursor.itemId, 1);
                Int32 newCursorCount = cursor.count - 1;
                player.setCarriedItem(newCursorCount > 0 ? cursor.itemId : -1, newCursorCount);
            }
            return;
        }

        if (target.itemId == cursor.itemId) {
            Int32 maxStack = ItemProperties::getMaxStackSize(target.itemId);
            if (button == 0) {
                Int32 room = maxStack - target.count;
                Int32 moved = std::min(room, cursor.count);
                if (moved > 0) {
                    player.setSlot(slot, target.itemId, target.count + moved);
                    Int32 remaining = cursor.count - moved;
                    player.setCarriedItem(remaining > 0 ? cursor.itemId : -1, remaining);
                }
            } else if (target.count < maxStack) {
                player.setSlot(slot, target.itemId, target.count + 1);
                Int32 remaining = cursor.count - 1;
                player.setCarriedItem(remaining > 0 ? cursor.itemId : -1, remaining);
            }
        } else {
            // Mismatched items -- full swap regardless of button.
            player.setSlot(slot, cursor.itemId, cursor.count);
            player.setCarriedItem(target.itemId, target.count);
        }
    }

    // Merges `count` of itemId into slots [start, end), existing partial
    // stacks first then empty slots. Returns leftover that didn't fit.
    Int32 MergeIntoRange(Player& player, Int32 itemId, Int32 count, int start, int end) {
        Int32 maxStack = ItemProperties::getMaxStackSize(itemId);
        for (int i = start; i < end && count > 0; i++) {
            InventorySlot s = player.getSlot(i);
            if (s.itemId != itemId) continue;
            Int32 room = maxStack - s.count;
            if (room <= 0) continue;
            Int32 moved = std::min(room, count);
            player.setSlot(i, itemId, s.count + moved);
            count -= moved;
        }
        for (int i = start; i < end && count > 0; i++) {
            InventorySlot s = player.getSlot(i);
            if (s.itemId != -1) continue;
            Int32 moved = std::min(maxStack, count);
            player.setSlot(i, itemId, moved);
            count -= moved;
        }
        return count;
    }

    // Real slot index for an EquipmentSlot (armor 5-8, offhand 45) -- None
    // has no single slot and is never queried here.
    int ArmorSlotIndex(EquipmentSlot equipmentSlot) {
        switch (equipmentSlot) {
            case EquipmentSlot::Head: return 5;
            case EquipmentSlot::Chest: return 6;
            case EquipmentSlot::Legs: return 7;
            case EquipmentSlot::Feet: return 8;
            case EquipmentSlot::Offhand: return Player::HOTBAR_START + Player::HOTBAR_SIZE; // 45
            default: return -1;
        }
    }

    // Mode 1: shift click -- quick-move the entire clicked stack. Armor and
    // shield route straight to their matching equipment slot if it's empty
    // (matches real vanilla's well-known shift-click-to-equip behavior);
    // everything else (and armor/shield when that slot is already occupied)
    // falls back to the complementary region (hotbar/offhand <-> main
    // storage). Slots 0-8 (crafting grid, no gameplay meaning yet) default to
    // main storage.
    void HandleShiftClick(Player& player, int slot) {
        if (slot < 0 || slot >= Player::TOTAL_SLOTS) return;
        InventorySlot source = player.getSlot(slot);
        if (source.itemId < 0) return;

        EquipmentSlot equipSlot = ItemProperties::getEquipmentSlot(source.itemId);
        int armorSlot = ArmorSlotIndex(equipSlot);
        if (armorSlot >= 0 && armorSlot != slot && player.getSlot(armorSlot).itemId < 0) {
            player.setSlot(armorSlot, source.itemId, source.count);
            player.setSlot(slot, -1, 0);
            return;
        }

        int targetStart, targetEnd;
        if (slot >= Player::HOTBAR_START) {
            targetStart = 9; targetEnd = Player::HOTBAR_START;
        } else if (slot >= 9) {
            targetStart = Player::HOTBAR_START; targetEnd = Player::HOTBAR_START + Player::HOTBAR_SIZE;
        } else {
            targetStart = 9; targetEnd = Player::HOTBAR_START;
        }

        Int32 leftover = MergeIntoRange(player, source.itemId, source.count, targetStart, targetEnd);
        if (leftover == 0) {
            player.setSlot(slot, -1, 0);
        } else if (leftover < source.count) {
            player.setSlot(slot, source.itemId, leftover);
        }
    }

    // Mode 2: number-key swap (button 0-8 -> hotbar index, 40 -> offhand) --
    // full swap with the clicked slot, no merging.
    void HandleNumberKeySwap(Player& player, int slot, int button) {
        if (slot < 0 || slot >= Player::TOTAL_SLOTS) return;
        int targetSlot;
        if (button >= 0 && button <= 8) {
            targetSlot = Player::HOTBAR_START + button;
        } else if (button == 40) {
            targetSlot = Player::HOTBAR_START + Player::HOTBAR_SIZE; // offhand slot (45)
        } else {
            return;
        }
        if (targetSlot == slot) return;
        InventorySlot a = player.getSlot(slot);
        InventorySlot b = player.getSlot(targetSlot);
        player.setSlot(slot, b.itemId, b.count);
        player.setSlot(targetSlot, a.itemId, a.count);
    }

    // Mode 4: drop key -- acts directly on the clicked slot, cursor untouched.
    // Button 0 = drop one (Q), button 1 = drop the entire stack (Ctrl+Q).
    void HandleDropClick(Player& player, int slot, int button) {
        if (slot < 0 || slot >= Player::TOTAL_SLOTS) return;
        InventorySlot source = player.getSlot(slot);
        if (source.itemId < 0 || source.count <= 0) return;
        if (button == 0) {
            SpawnDroppedItemEntity(player, source.itemId, 1);
            Int32 newCount = source.count - 1;
            player.setSlot(slot, newCount > 0 ? source.itemId : -1, newCount);
        } else {
            SpawnDroppedItemEntity(player, source.itemId, source.count);
            player.setSlot(slot, -1, 0);
        }
    }

    // Mode 6: double-click collect -- scans the whole inventory for stacks
    // matching the cursor's item, folding them into the cursor up to its
    // real max stack size. Button 1 ("pickup all but check items in reverse
    // order") is documented as impossible for a real vanilla client to send.
    void HandleDoubleClick(Player& player, int button) {
        if (button != 0) return;
        InventorySlot cursor = player.getCarriedItem();
        if (cursor.itemId < 0) return;
        Int32 needed = ItemProperties::getMaxStackSize(cursor.itemId) - cursor.count;
        if (needed <= 0) return;
        for (int i = 0; i < Player::TOTAL_SLOTS && needed > 0; i++) {
            InventorySlot s = player.getSlot(i);
            if (s.itemId != cursor.itemId) continue;
            Int32 taken = std::min(s.count, needed);
            if (taken <= 0) continue;
            player.setSlot(i, taken == s.count ? -1 : s.itemId, s.count - taken);
            needed -= taken;
            cursor.count += taken;
        }
        player.setCarriedItem(cursor.itemId, cursor.count);
    }
}

void Click_Container_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Click_Container_p::deserialize(): Received.");
    #endif
    Byte windowId = deserializeByte(in_buff);
    Int32 stateId = deserializeVarInt(in_buff);
    Int16 slot = deserializeShort(in_buff);
    Byte button = deserializeByte(in_buff);
    Int32 mode = deserializeVarInt(in_buff);
    // The trailing array-of-changed-slots and carried-item fields are
    // deliberately never parsed -- every result above is computed purely
    // server-side from Mode/Button/Slot plus the player's own already-tracked
    // state, never trusting client-claimed values. Safe to leave unconsumed:
    // this packet's own outer length prefix already demarcates its exact
    // byte range before dispatch (same established convention documented on
    // unpackSlot above).

    Player& player = cont.connection.getPlayer();
    int threshold = cont.connection.getCompressionThreshold();

    if (windowId != 0) return; // only the player inventory window exists in this project

    if (stateId != player.getContainerStateId()) {
        // Stale/desynced client view -- ignore the click and resync fully,
        // exactly matching docs/network-protocol.md's documented behavior.
        cont.connection.addPacket(std::make_shared<Set_Container_Content_p>(
            threshold, player.getInventory(), player.advanceContainerStateId(), player.getCarriedItem()));
        return;
    }

    switch (mode) {
        case 0: HandleNormalClick(player, slot, button); break;
        case 1: HandleShiftClick(player, slot); break;
        case 2: HandleNumberKeySwap(player, slot, button); break;
        case 3: break; // middle-click clone -- only meaningful in non-player-inventory windows, which don't exist in this project
        case 4: HandleDropClick(player, slot, button); break;
        case 5:
            // *** CHECKLIST: drag/paint-dragging across multiple slots ***
            // Real vanilla sends this as a 3-packet sequence (start at Slot
            // -999 with button 0/4/8 for left/right/middle, one packet per
            // painted slot with button 1/5/9, end at Slot -999 with button
            // 2/6/10) -- needs a real per-player "in-progress drag" session
            // (accumulated slot list + which button started it) tracked
            // across those packets, a genuine scope jump from every other
            // mode here (each fully resolved within its own single packet).
            // Deliberately a no-op until that's built.
            break;
        case 6: HandleDoubleClick(player, button); break;
        default: break;
    }

    // Always resync the full inventory + cursor rather than diff exactly
    // which slots each mode touched (several above can touch anywhere from 1
    // to all 46 slots) -- simpler and safer than per-slot bookkeeping, at a
    // bandwidth cost that's negligible for a hobby server.
    cont.connection.addPacket(std::make_shared<Set_Container_Content_p>(
        threshold, player.getInventory(), player.advanceContainerStateId(), player.getCarriedItem()));
}

void Set_Held_Item_serverbound_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Set_Held_Item_serverbound_p::deserialize(): Received.");
    #endif
    Int16 slot = deserializeShort(in_buff);
    cont.connection.getPlayer().setSelectedSlot(slot);
}