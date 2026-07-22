#include <Standards.hpp>
#include <network/packets/Config.hpp>
#include <network/packets/Play.hpp>
#include <network/PacketUtils.hpp>
#include <network/Connection.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <Player.hpp>
#include <World.hpp>
#include <ItemBlockMapping.hpp>
#include <PlayerDataPersistence.hpp>
#include <OpsList.hpp>
#include <Console.hpp>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

std::vector<Byte> Cookie_Request_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Clientbound_Plugin_Message_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Disconnect_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

Finish_Config_p::Finish_Config_p(int threshold) {
    _threshold = threshold;
}

std::vector<Byte> Finish_Config_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Finish_Config_p::serialize(): Sending.");
    #endif
    return assemblePacket(getID(), _threshold, std::vector<Byte>());
}

Clientbound_Keep_Alive_config_p::Clientbound_Keep_Alive_config_p(int threshold, Int64 keepAliveId) {
    _threshold = threshold;
    _keepAliveId = keepAliveId;
}

std::vector<Byte> Clientbound_Keep_Alive_config_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Clientbound_Keep_Alive_config_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    for (int i = 7; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_keepAliveId >> (i * 8)) & 0xFF));
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

Ping_config_p::Ping_config_p(int threshold, Int32 pingId) {
    _threshold = threshold;
    _pingId = pingId;
}

std::vector<Byte> Ping_config_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Ping_config_p::serialize(): Sending.");
    #endif
    std::vector<Byte> packet_data;
    for (int i = 3; i >= 0; i--) {
        packet_data.push_back(static_cast<Byte>((_pingId >> (i * 8)) & 0xFF));
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

std::vector<Byte> Reset_Chat_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

Registry_Data_p::Registry_Data_p(int threshold, const std::string& registryId, std::vector<RegistryEntry> entries) {
    _threshold = threshold;
    _registryId = registryId;
    _entries = std::move(entries);
}

std::vector<Byte> Registry_Data_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Registry_Data_p::serialize(): Sending " + _registryId);
    #endif
    std::vector<Byte> packet_data = serializeString(_registryId);
    std::vector<Byte> count = varIntSerialize(static_cast<int>(_entries.size()));
    packet_data.insert(packet_data.end(), count.begin(), count.end());
    for (const auto& entry : _entries) {
        std::vector<Byte> idBytes = serializeString(entry.id);
        packet_data.insert(packet_data.end(), idBytes.begin(), idBytes.end());
        packet_data.push_back(entry.hasData ? 0x01 : 0x00);
        if (entry.hasData) {
            std::vector<Byte> nbtBytes = entry.data.serializeNetwork();
            packet_data.insert(packet_data.end(), nbtBytes.begin(), nbtBytes.end());
        }
    }
    return assemblePacket(getID(), _threshold, packet_data);
}

std::vector<Byte> Remove_Resource_Pack_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Add_Resource_Pack_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Store_Cookie_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Transfer_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Feature_Flags_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

Update_Tags_config_p::Update_Tags_config_p(int threshold) {
    _threshold = threshold;
}

std::vector<Byte> Update_Tags_config_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Update_Tags_config_p::serialize(): Sending.");
    #endif
    // No tag registries yet; an empty array is valid and lets the client proceed.
    std::vector<Byte> packet_data = varIntSerialize(0);
    return assemblePacket(getID(), _threshold, packet_data);
}

Clientbound_Known_Packs_p::Clientbound_Known_Packs_p(int threshold) {
    _threshold = threshold;
}

std::vector<Byte> Clientbound_Known_Packs_p::serialize() const {
    #ifdef DEBUG
        Console::getConsole().Entry("Clientbound_Known_Packs_p::serialize(): Sending.");
    #endif
    // We only advertise the vanilla "core" data pack, so the client always
    // has to accept the Registry Data we send afterward rather than skip it.
    std::vector<Byte> packet_data = varIntSerialize(1);
    std::vector<Byte> ns = serializeString("minecraft");
    std::vector<Byte> id = serializeString("core");
    std::vector<Byte> version = serializeString(SERVER_VERSION);
    packet_data.insert(packet_data.end(), ns.begin(), ns.end());
    packet_data.insert(packet_data.end(), id.begin(), id.end());
    packet_data.insert(packet_data.end(), version.begin(), version.end());
    return assemblePacket(getID(), _threshold, packet_data);
}

std::vector<Byte> Custom_Report_Details_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

std::vector<Byte> Server_Links_config_p::serialize() const {
    // TODO Implementation here
    return std::vector<Byte>();
}

void Client_Information_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Client_Information_config_p::deserialize(): Received.");
    #endif
    // Locale (String): unused for now.
    deserializeString(in_buff);
    Byte viewDistance = deserializeByte(in_buff);
    deserializeVarInt(in_buff); // Chat Mode: unused, no chat implemented yet.
    deserializeBool(in_buff); // Chat Colors: unused.
    Byte skinParts = deserializeByte(in_buff);
    // Main Hand, text filtering, server listings: unused.

    cont.connection.getPlayer().setViewDistance(static_cast<int>(viewDistance));
    cont.connection.getPlayer().setSkinParts(skinParts);
}

void Cookie_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Serverbound_Plugin_Message_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Acknowledge_Finish_Config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Acknowledge_Finish_Config_p::deserialize(): Received, switching to Play state.");
    #endif
    cont.connection.setState(ConnectionState::Play);

    Player& player = cont.connection.getPlayer();
    // Tab-list sync: tell everyone else about this player, and this player
    // about everyone else. Separate from in-world visibility (PlayerVisibilityManager),
    // which is chunk/proximity-based and handled once chunks start delivering.
    BroadcastPlayerJoin(cont, player);
    World& world = World::getInstance();
    // A returning player resumes exactly where they left off (position,
    // rotation, gamemode, hotbar) -- see PlayerDataPersistence. A brand-new
    // UUID falls back to the world spawn point and a starting test stack, as
    // this project always did before per-player persistence existed.
    std::optional<PlayerSaveData> saved = PlayerDataPersistence::tryLoad(world.getWorldDir(), player.getUUID());
    if (saved) {
        player.setPosition(saved->x, saved->y, saved->z);
        player.setRotation(saved->yaw, saved->pitch);
        player.setGamemode(saved->gamemode);
        for (int i = 0; i < Player::HOTBAR_SIZE; i++) {
            player.setHotbarSlot(i, saved->hotbar[i].itemId, saved->hotbar[i].count);
        }
        player.setSelectedSlot(saved->selectedSlot);
    } else {
        player.setPosition(world.getSpawnX(), world.getSpawnY(), world.getSpawnZ());
        player.setRotation(world.getSpawnYaw(), 0.0f);
        player.setGamemode(0);
        // Seed the hotbar with a placeable test stack so breaking/placing is
        // immediately testable without any survival-mode item pickup (which
        // isn't implemented -- see docs/general-documentation.md, "Minimal inventory").
        player.setHotbarSlot(0, STONE_ITEM_ID, 64);
    }

    // Guarantee the player's own standing chunk is generated/loaded + lit +
    // cached BEFORE they're teleported there -- previously invisible since
    // every join was at the pre-warmed spawn chunk (0,0), primed synchronously
    // in World's constructor. A restored (non-spawn) position is a cache
    // miss, and without this, the client would receive the teleport before
    // its chunk was even queued for async generation, free-falling through
    // an unloaded column until the real terrain caught up a moment later.
    int homeChunkX = static_cast<int>(std::floor(player.getX() / 16.0));
    int homeChunkZ = static_cast<int>(std::floor(player.getZ() / 16.0));
    world.ensureChunkLoaded(homeChunkX, homeChunkZ);

    int threshold = cont.connection.getCompressionThreshold();
    // The client's own third-person model (F5) reads its skin-parts bitmask
    // from its own entity's tracked metadata too, same as the skin texture
    // itself needed Player_Info_Update_p sent back to the joiner -- it isn't
    // purely local rendering the way it might seem. PlayerVisibilityManager
    // never sends this to a player about themselves (a player's own entity is
    // never "spawned" to them), so it has to happen here instead.
    cont.connection.addPacket(std::make_shared<Set_Player_Skin_Parts_Metadata_p>(threshold, player.getEntityId(), player.getSkinParts()));
    std::shared_ptr<Outgoing_Packet> loginPlay = std::make_shared<Login_Play_p>(threshold, player);
    cont.connection.addPacket(loginPlay);
    std::shared_ptr<Outgoing_Packet> defaultSpawn = std::make_shared<Set_Default_Spawn_Position_p>(threshold);
    cont.connection.addPacket(defaultSpawn);
    // Only one teleport is ever in flight today, so a fixed ID is enough;
    // Confirm_Teleportation_p doesn't validate it yet either.
    std::shared_ptr<Outgoing_Packet> syncPosition = std::make_shared<Synchronize_Player_Position_p>(
        threshold, player.getX(), player.getY(), player.getZ(), player.getYaw(), player.getPitch(), 0);
    cont.connection.addPacket(syncPosition);
    cont.connection.addPacket(std::make_shared<Set_Container_Content_p>(threshold, player.getHotbar()));
    // Same permission lookup PlayerCommandSender::getPermissionLevel() does --
    // reused directly rather than constructing a full CommandSender just to
    // read one int.
    int permissionLevel = OpsList::getInstance().getOpLevel(uuidToHexString(player.getUUID()));
    cont.connection.addPacket(std::make_shared<Commands_p>(threshold, permissionLevel));
    // Sent once more here so the sky looks correct immediately at join,
    // instead of waiting up to a second for DayNightSystem's first periodic
    // broadcast.
    cont.connection.addPacket(std::make_shared<Update_Time_p>(threshold, world.getDayTime()));

    // Send every chunk within the player's view distance around their own
    // (real, possibly-restored) position, not always spawn -- see homeChunkX/Z
    // above. Movement-triggered updates (Set_Player_Position_p etc. in
    // Play.cpp) diff against whatever UpdateLoadedChunks records here.
    std::shared_ptr<Outgoing_Packet> gameEvent = std::make_shared<Game_Event_p>(threshold, 13, 0.0f); // Start waiting for level chunks
    cont.connection.addPacket(gameEvent);
    UpdateLoadedChunks(cont, threshold, player, homeChunkX, homeChunkZ);
}

void Serverbound_Keep_Alive_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Serverbound_Keep_Alive_config_p::deserialize(): Received.");
    #endif
    // TODO: validate against the ID we last sent once Connection proactively
    // sends Keep Alives on a timer; nothing does yet, so there's nothing to check.
}

void Pong_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Pong_config_p::deserialize(): Received.");
    #endif
    // TODO: validate against the ID we last sent once Connection proactively
    // sends Pings on a timer; nothing does yet, so there's nothing to check.
}

void Resource_Pack_Response_config_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    // TODO Implementation here
}

void Serverbound_Known_Packs_p::deserialize(std::vector<Byte> in_buff, PacketContext& cont) {
    #ifdef DEBUG
        Console::getConsole().Entry("Serverbound_Known_Packs_p::deserialize(): Received.");
    #endif
    // We don't branch on which packs the client already knows; we always send
    // our full Registry Data regardless, which is always valid per protocol.
    int threshold = cont.connection.getCompressionThreshold();
    VanillaDataManager& registries = VanillaDataManager::getInstance();
    for (const string& registryName : registries.getRegistryNames()) {
        const std::vector<RegistryEntry>& entries = registries.getEntries(registryName);
        if (entries.empty()) {
            continue; // missing/failed-to-load registry; skip rather than send an empty one
        }
        std::shared_ptr<Outgoing_Packet> registryPacket = std::make_shared<Registry_Data_p>(threshold, "minecraft:" + registryName, entries);
        cont.connection.addPacket(registryPacket);
    }
    std::shared_ptr<Outgoing_Packet> tagsPacket = std::make_shared<Update_Tags_config_p>(threshold);
    cont.connection.addPacket(tagsPacket);
    std::shared_ptr<Outgoing_Packet> finishPacket = std::make_shared<Finish_Config_p>(threshold);
    cont.connection.addPacket(finishPacket);
}