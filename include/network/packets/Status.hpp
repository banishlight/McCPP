#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>

// **Status Packets**

class Status_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        Status_Response_p(int threshold);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Pong_Response_p : public Status_Packet, public Outgoing_Packet {
    public:
        Pong_Response_p(int threshold, long timestamp);
        int getID() const override { return _PACKET_ID; }
        std::vector<Byte> serialize() const override;
    private:
        long _timestamp;
        static int constexpr _PACKET_ID = 0x01;
};

class Status_Request_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x00;
};
class Ping_Request_status_p : public Status_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
        static int constexpr _PACKET_ID = 0x01;
};