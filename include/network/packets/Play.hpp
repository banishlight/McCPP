#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>

// Forward declaration
class Player;

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

class Confirm_Teleportation_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Serverbound_Keep_Alive_play_p : public Play_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x18;
};