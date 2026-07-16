#pragma once
#include <Standards.hpp>
#include <network/Packet.hpp>

// **Handshake Packets**

class Handshake_p : public Handshake_Packet, public Incoming_Packet {
    public:
        int getID() const override { return _PACKET_ID; }
        void deserialize(std::vector<Byte> in_buff, PacketContext& cont) override;
    private:
	    static int constexpr _PACKET_ID = 0x00;
};