#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>
#include <network/PacketContext.hpp>
#include <Player.hpp>
#include <memory>
#include <functional>
#include <array>

// Forward declaration
class Player;
class Chunk;
class World;

// **Play Packets**

class Login_Play_p : public Play_Packet, public Outgoing_Packet {
    public:
        Login_Play_p(int threshold, const Player& player);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        int _gamemode;
        int _viewDistance;
        static int constexpr _PACKET_ID = 0x2B;
};
class Synchronize_Player_Position_p : public Play_Packet, public Outgoing_Packet {
    public:
        Synchronize_Player_Position_p(int threshold, double x, double y, double z, float yaw, float pitch, int teleportId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        double _x;
        double _y;
        double _z;
        float _yaw;
        float _pitch;
        int _teleportId;
        static int constexpr _PACKET_ID = 0x40;
};
class Set_Default_Spawn_Position_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Default_Spawn_Position_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x56;
};
// Advertises every command as a bare executable literal node (root -> one
// child per command, no argument children) so the client autocompletes/
// highlights them in the chat box -- see docs/general-documentation.md,
// "Command autocomplete", for why argument nodes are deliberately out of
// scope (the Brigadier parser-ID table is version-fragile and unverified).
class Commands_p : public Play_Packet, public Outgoing_Packet {
    public:
        // permissionLevel filters out commands the joining player can't run,
        // mirroring HelpCommand's own existing filter (same convention, not
        // a new design decision).
        Commands_p(int threshold, int permissionLevel);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        // Flat node array (children referenced by index into this same
        // vector), built once in the constructor -- serialize() just walks
        // it. Node 0 is always the root (empty name). A command with no
        // Command::getArgumentSuggestions() is one executable, childless
        // literal node (the original, still-most-common shape); a command
        // with suggestions gets a non-executable node whose children are
        // one executable literal per suggestion (see GamemodeCommand for
        // the one command using this today).
        struct Node {
            Byte flags;
            std::vector<int> children;
            string name; // empty only for the root
        };
        std::vector<Node> _nodes;
        static int constexpr _PACKET_ID = 0x11;
};
class Clientbound_Keep_Alive_play_p : public Play_Packet, public Outgoing_Packet {
    public:
        Clientbound_Keep_Alive_play_p(int threshold, Int64 keepAliveId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int64 _keepAliveId;
        static int constexpr _PACKET_ID = 0x26;
};
// Drives the client's day/night sky/sun rendering. No /time set command
// exists in this project, so World Age and Time of day are always the same
// value (matches LevelDat's own existing DayTime/Time conflation) -- one
// param, sent for both wire fields. See DayNightSystem for the periodic
// broadcast and World::advanceDayTime for the underlying counter.
class Update_Time_p : public Play_Packet, public Outgoing_Packet {
    public:
        Update_Time_p(int threshold, Int64 dayTime);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int64 _dayTime;
        static int constexpr _PACKET_ID = 0x64;
};
class Game_Event_p : public Play_Packet, public Outgoing_Packet {
    public:
        Game_Event_p(int threshold, Byte event, float value);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Byte _event;
        float _value;
        static int constexpr _PACKET_ID = 0x22;
};
class Set_Center_Chunk_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Center_Chunk_p(int threshold, int chunkX, int chunkZ);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _chunkX;
        int _chunkZ;
        static int constexpr _PACKET_ID = 0x54;
};
class Chunk_Data_p : public Play_Packet, public Outgoing_Packet {
    public:
        Chunk_Data_p(int threshold, std::shared_ptr<Chunk> chunk);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::shared_ptr<Chunk> _chunk;
        static int constexpr _PACKET_ID = 0x27;
};
class Unload_Chunk_p : public Play_Packet, public Outgoing_Packet {
    public:
        Unload_Chunk_p(int threshold, int chunkX, int chunkZ);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _chunkX;
        int _chunkZ;
        static int constexpr _PACKET_ID = 0x21;
};
class Block_Update_p : public Play_Packet, public Outgoing_Packet {
    public:
        Block_Update_p(int threshold, Int32 x, Int32 y, Int32 z, Int32 blockStateId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int32 _x, _y, _z;
        Int32 _blockStateId;
        static int constexpr _PACKET_ID = 0x09;
};
class Acknowledge_Block_Change_p : public Play_Packet, public Outgoing_Packet {
    public:
        Acknowledge_Block_Change_p(int threshold, int sequence);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _sequence;
        static int constexpr _PACKET_ID = 0x05;
};
class Set_Container_Content_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Container_Content_p(int threshold, const std::array<HotbarSlot, Player::HOTBAR_SIZE>& hotbar);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::array<HotbarSlot, Player::HOTBAR_SIZE> _hotbar; // snapshot, not a Player& -- serialize() may run later, off the caller's thread
        static int constexpr _PACKET_ID = 0x13;
};
class Set_Container_Slot_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Container_Slot_p(int threshold, int slotIndex, Int32 itemId, Int32 count);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _slotIndex;
        Int32 _itemId, _count;
        static int constexpr _PACKET_ID = 0x15;
};
class Spawn_Entity_p : public Play_Packet, public Outgoing_Packet {
    public:
        // yaw/pitch/headYaw default to 0 so existing item-entity call sites
        // (which have no meaningful rotation) are unaffected. data defaults to
        // 0 ("unused" per docs/general-documentation.md's Object Data note)
        // -- Falling Block is the only entity type this project spawns that
        // needs a nonzero value (the block state ID it represents).
        Spawn_Entity_p(int threshold, int entityId, const std::vector<long>& uuid, int entityTypeId, double x, double y, double z,
                       float yaw = 0.0f, float pitch = 0.0f, float headYaw = 0.0f, Int32 data = 0);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        std::vector<long> _uuid;
        int _entityTypeId;
        double _x, _y, _z;
        float _yaw, _pitch, _headYaw;
        Int32 _data;
        static int constexpr _PACKET_ID = 0x01;
};
class Set_Entity_Metadata_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Entity_Metadata_p(int threshold, int entityId, Int32 itemId, Int32 count);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        Int32 _itemId, _count;
        static int constexpr _PACKET_ID = 0x58;
};
class Set_Player_Skin_Parts_Metadata_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Player_Skin_Parts_Metadata_p(int threshold, int entityId, Byte skinParts);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        Byte _skinParts;
        static int constexpr _PACKET_ID = 0x58;
};
// Entity Flags, index 0, Byte bitmask (0x02 crouching, 0x08 sprinting) --
// on the base Entity class, stable across versions (unlike the Player-specific
// skin-parts field). Sent as its own packet, not combined with Pose below --
// these must go out as two separate Set Entity Metadata packets, never
// combined into one, or the pose change silently fails to render.
class Set_Entity_Flags_Metadata_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Entity_Flags_Metadata_p(int threshold, int entityId, bool sneaking, bool sprinting);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        bool _sneaking, _sprinting;
        static int constexpr _PACKET_ID = 0x58;
};
// Pose, index 6, VarInt enum (STANDING=0, SNEAKING=5) -- also on the base
// Entity class. Kept as its own packet, sent alongside (not combined with)
// Set_Entity_Flags_Metadata_p -- see that class's comment.
class Set_Player_Pose_Metadata_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Player_Pose_Metadata_p(int threshold, int entityId, bool sneaking);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        bool _sneaking;
        static int constexpr _PACKET_ID = 0x58;
};
class Remove_Entities_p : public Play_Packet, public Outgoing_Packet {
    public:
        Remove_Entities_p(int threshold, int entityId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        static int constexpr _PACKET_ID = 0x42;
};
class Pickup_Item_p : public Play_Packet, public Outgoing_Packet {
    public:
        Pickup_Item_p(int threshold, int collectedEntityId, int collectorEntityId, int count);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _collectedEntityId, _collectorEntityId, _count;
        static int constexpr _PACKET_ID = 0x6F;
};
class Set_Entity_Velocity_p : public Play_Packet, public Outgoing_Packet {
    public:
        // vx/vy/vz in blocks/tick -- converted internally to the wire's
        // 1/8000-block-per-tick fixed-point units (docs/network-protocol.md).
        Set_Entity_Velocity_p(int threshold, int entityId, double vx, double vy, double vz);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        double _vx, _vy, _vz;
        static int constexpr _PACKET_ID = 0x5A;
};
class Teleport_Entity_p : public Play_Packet, public Outgoing_Packet {
    public:
        Teleport_Entity_p(int threshold, int entityId, double x, double y, double z, float yaw, float pitch, bool onGround);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        double _x, _y, _z;
        float _yaw, _pitch;
        bool _onGround;
        static int constexpr _PACKET_ID = 0x70;
};
class Update_Entity_Position_p : public Play_Packet, public Outgoing_Packet {
    public:
        // deltaX/Y/Z: fixed-point, 4096 units/block (currentPos*4096 - prevPos*4096).
        Update_Entity_Position_p(int threshold, int entityId, Int16 deltaX, Int16 deltaY, Int16 deltaZ, bool onGround);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        Int16 _deltaX, _deltaY, _deltaZ;
        bool _onGround;
        static int constexpr _PACKET_ID = 0x2E;
};
class Update_Entity_Position_and_Rotation_p : public Play_Packet, public Outgoing_Packet {
    public:
        Update_Entity_Position_and_Rotation_p(int threshold, int entityId, Int16 deltaX, Int16 deltaY, Int16 deltaZ, float yaw, float pitch, bool onGround);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        Int16 _deltaX, _deltaY, _deltaZ;
        float _yaw, _pitch;
        bool _onGround;
        static int constexpr _PACKET_ID = 0x2F;
};
class Update_Entity_Rotation_p : public Play_Packet, public Outgoing_Packet {
    public:
        Update_Entity_Rotation_p(int threshold, int entityId, float yaw, float pitch, bool onGround);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        float _yaw, _pitch;
        bool _onGround;
        static int constexpr _PACKET_ID = 0x30;
};
class Set_Head_Rotation_p : public Play_Packet, public Outgoing_Packet {
    public:
        Set_Head_Rotation_p(int threshold, int entityId, float headYaw);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        float _headYaw;
        static int constexpr _PACKET_ID = 0x48;
};
// Server-wide tab list (not proximity-based -- every connected player is told
// about every other one, regardless of distance, matching vanilla). Implements
// Add Player (0x01), Update Game Mode (0x04), and Update Listed (0x08).
// Latency (0x10) isn't tracked by this server yet -- no round-trip Keep Alive
// timing exists to source a real ping from. Listed is always sent true: this
// project has no mechanism to hide a connected player from the tab list.
class Player_Info_Update_p : public Play_Packet, public Outgoing_Packet {
    public:
        struct Entry {
            std::vector<long> uuid;
            std::string name;
            std::vector<PlayerProfileProperty> properties;
            int gamemode = 0;
        };
        Player_Info_Update_p(int threshold, const std::vector<Entry>& entries);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::vector<Entry> _entries;
        static int constexpr _PACKET_ID = 0x3E;
};
class Player_Info_Remove_p : public Play_Packet, public Outgoing_Packet {
    public:
        Player_Info_Remove_p(int threshold, const std::vector<std::vector<long>>& uuids);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        std::vector<std::vector<long>> _uuids;
        static int constexpr _PACKET_ID = 0x3D;
};

// Sent before closing a Play-state connection server-side (kick, /stop) so
// the client knows why -- the client is expected to close its own end on
// receipt, which is what actually makes Connection::isValid() go false
// afterward (see Connection::disconnect()); this server never force-closes
// the socket itself.
class Disconnect_play_p : public Play_Packet, public Outgoing_Packet {
    public:
        Disconnect_play_p(int threshold, const string& reason);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        string _reason;
        static int constexpr _PACKET_ID = 0x1D;
};

// A chat message with no signing information -- what vanilla itself sends for
// console/command-originated chat (/say, /tell, /me), which is exactly this
// server's use case since it never implements chat signing. chatTypeId is a
// minecraft:chat_type registry identifier (e.g. "minecraft:chat",
// "minecraft:say_command"); its wire index is looked up at send time since
// registry send order isn't fixed (see BroadcastDisguisedChat).
class Disguised_Chat_Message_p : public Play_Packet, public Outgoing_Packet {
    public:
        Disguised_Chat_Message_p(int threshold, const string& message, int chatTypeIndex, const string& senderName);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        string _message;
        int _chatTypeIndex;
        string _senderName;
        static int constexpr _PACKET_ID = 0x1E;
};

// A raw system message (command feedback, errors) -- not associated with any
// player, so no chat_type/sender is needed.
class System_Chat_Message_p : public Play_Packet, public Outgoing_Packet {
    public:
        System_Chat_Message_p(int threshold, const string& message, bool overlay = false);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        string _message;
        bool _overlay;
        static int constexpr _PACKET_ID = 0x6C;
};

// Sends a Disguised_Chat_Message_p (chatTypeId in the minecraft:chat_type
// registry, e.g. "minecraft:chat" for player chat, "minecraft:say_command"
// for /say) to every active Play connection.
void BroadcastDisguisedChat(const string& senderName, const string& message, const string& chatTypeId);

// Shared by the initial Configuration->Play chunk send and every subsequent
// movement-triggered update: diffs the player's currently-loaded chunk set
// against the (2*viewDistance+1) square centered on (newCenterX, newCenterZ),
// sending Chunk_Data_p/Unload_Chunk_p only for what actually changed.
void UpdateLoadedChunks(PacketContext& cont, int threshold, Player& player, int newCenterX, int newCenterZ);

// Sends one packet (freshly constructed per recipient, since compression
// threshold is per-connection) to every active connection whose player
// currently has (chunkX, chunkZ) loaded -- used for block edits, which must
// reach every nearby player, not just whoever triggered them.
void BroadcastToChunkViewers(int chunkX, int chunkZ, const std::function<std::shared_ptr<Outgoing_Packet>(int threshold)>& makePacket);

// Checks whether the block at (x,y,z) is gravity-affected (sand/gravel) and
// resting on nothing -- if so, removes it from the static world and spawns a
// FallingBlockEntity in its place (see FallingBlockSystem for the actual
// fall/landing simulation). Call after any world edit that could pull support
// out from under a block above it (breaking, placing into open air) or place
// a gravity block directly onto open air. Recurses upward through stacked
// gravity blocks, mirroring vanilla's chain-reaction collapse -- removing one
// block is itself an edit that can unsupport the block above it.
void CheckGravityBlock(World& world, int x, int y, int z);

// Called after every position update (matches vanilla: the Notchian server
// only checks for pickups after Set Player Position/Set Player Position And
// Rotation). Scans nearby tracked item entities and, for any the player has
// hotbar room for, claims and collects it.
void TryPickupNearbyItems(PacketContext& cont, int threshold, Player& player);

// Server-wide tab-list sync for a newly-joined player: broadcasts an Add
// Player entry for them to every other already-Play connection, and sends
// them one Player_Info_Update_p covering every existing player. Called once,
// right after a connection enters Play state.
void BroadcastPlayerJoin(PacketContext& cont, Player& joiningPlayer);

// Grants (or revokes) the client-side ability to fly/instant-break for a
// given gamemode -- used both at join and by /gamemode, see Player_Abilities_p.
// Creative grants Allow Flying but not Flying itself (matches vanilla: the
// client's own double-tap-space gesture toggles actual flight once allowed).
// Spectator grants Flying directly, since spectator always flies/noclips.
Byte abilitiesFlagsForGamemode(int gamemode);

// Grants/revokes fly and instant-break client-side -- without this, switching
// to Creative changes the hotbar/tab-list icon but the client still can't
// actually fly or instant-break, since those are gated behind these flags,
// not anything Game_Event_p's "Change game mode" event alone conveys.
class Player_Abilities_p : public Play_Packet, public Outgoing_Packet {
    public:
        Player_Abilities_p(int threshold, Byte flags, float flyingSpeed = 0.05f, float fovModifier = 0.1f);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Byte _flags;
        float _flyingSpeed;
        float _fovModifier;
        static int constexpr _PACKET_ID = 0x38; // clientbound -- a distinct namespace from the existing serverbound Use_Item_On_p at the same number
};

class Confirm_Teleportation_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Set_Player_Position_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x1A;
};
class Set_Player_Position_and_Rotation_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x1B;
};
class Set_Player_Rotation_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x1C;
};
class Serverbound_Keep_Alive_play_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x18;
};
class Player_Action_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x24;
};
class Player_Command_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x25;
};
// Regular chat (no leading "/"). Signature fields are consumed but never
// verified -- this server never implements chat signing (see
// Disguised_Chat_Message_p), matching how it already trusts online-mode
// verification instead of client-side message signatures for authenticity.
class Chat_Message_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x06;
};
// A "/command" typed in chat. Only the unsigned variant (0x04) is handled --
// the client only sends the signed variant (0x05) for arguments a Declare
// Commands packet marked as requiring signing, and this server never sends
// one, so real clients always take this path.
class Chat_Command_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x04;
};
class Use_Item_On_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x38;
};
// Sent by the client while the Creative-mode inventory screen is open --
// picking an item from the (entirely client-side, never sent by the server)
// creative tabs and placing it into a hotbar slot, or dragging it out of the
// window entirely. Structurally separate from Click Container (still out of
// scope, see docs/general-documentation.md) -- this is a narrower, distinct
// mechanism real vanilla uses specifically for creative-menu interactions.
class Set_Creative_Mode_Slot_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x32;
};
class Set_Held_Item_serverbound_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x2F;
};