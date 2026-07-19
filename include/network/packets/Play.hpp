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
class Clientbound_Keep_Alive_play_p : public Play_Packet, public Outgoing_Packet {
    public:
        Clientbound_Keep_Alive_play_p(int threshold, Int64 keepAliveId);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        Int64 _keepAliveId;
        static int constexpr _PACKET_ID = 0x26;
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
        Spawn_Entity_p(int threshold, int entityId, const std::vector<long>& uuid, int entityTypeId, double x, double y, double z);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        int _entityId;
        std::vector<long> _uuid;
        int _entityTypeId;
        double _x, _y, _z;
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
class Use_Item_On_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x38;
};
class Set_Held_Item_serverbound_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x2F;
};