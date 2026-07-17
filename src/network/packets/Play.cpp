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

void Confirm_Teleportation_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Confirm_Teleportation_p::deserialize(): Received.");
    #endif
    // TODO: validate against the teleport ID we last sent once multiple
    // in-flight teleports need disambiguating; only one is ever sent today.
}

void Serverbound_Keep_Alive_play_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Serverbound_Keep_Alive_play_p::deserialize(): Received.");
    #endif
    // TODO: validate against the ID we last sent once Connection proactively
    // sends Keep Alives on a timer; nothing does yet, so there's nothing to check.
}