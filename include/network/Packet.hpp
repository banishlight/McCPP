#pragma once
#include <Standards.hpp>
#include <network/PacketContext.hpp>
#include <memory>
#include <vector>

// Packet Format:
// Length (VarInt)
// Packet ID (VarInt)
// Data (Byte Array)
class Packet {
    public:
        virtual ~Packet() = default;
        virtual ConnectionState getState() const = 0;
        virtual int getID() const = 0;
};

class Incoming_Packet : public virtual Packet {
    public:
        virtual void deserialize(std::vector<Byte> in_buff, PacketContext& cont) = 0;
};
// Every Constructor must atleast contain the compression threshold
class Outgoing_Packet : public virtual Packet {
    public:
        virtual std::vector<Byte> serialize() const = 0;
    protected:
        int _threshold = -1;
};

class Handshake_Packet : public virtual Packet {
    public:
        ConnectionState getState() const override { return ConnectionState::Handshake; }
};
class Status_Packet : public virtual Packet {
    public:
        ConnectionState getState() const override { return ConnectionState::Status; }
};
class Login_Packet : public virtual Packet {
    public:
        ConnectionState getState() const override { return ConnectionState::Login; }
};
class Config_Packet : public virtual Packet {
    public:
        ConnectionState getState() const override { return ConnectionState::Config; }
};
class Play_Packet : public virtual Packet {
    public:
        ConnectionState getState() const override { return ConnectionState::Play; }
};

// *Packet Registry*
// Constructs one instance of every concrete packet class (across all connection
// states) to dispatch incoming packets by state + ID. See packets/*.hpp for the
// per-state packet class declarations this assembles.
class Packet_Registry {
    public:
        static Packet_Registry& getInstance();
        std::shared_ptr<Incoming_Packet> fetchIncomingPacket(ConnectionState state, int packetID);
    private:
        Packet_Registry();
        ~Packet_Registry() = default;
        void initializeRegistry();
        std::vector<std::vector<std::shared_ptr<Incoming_Packet>>> Incoming_Registry;
        std::vector<std::shared_ptr<Incoming_Packet>> HandshakeVec;
        std::vector<std::shared_ptr<Incoming_Packet>> StatusVec;
        std::vector<std::shared_ptr<Incoming_Packet>> LoginVec;
        std::vector<std::shared_ptr<Incoming_Packet>> ConfigVec;
        std::vector<std::shared_ptr<Incoming_Packet>> PlayVec;
        int const INC_SIZE = 5;
        int const HANDSHAKE_SIZE = 1;
        int const STATUS_SIZE = 2;
        int const LOGIN_SIZE = 5;
        int const CONFIG_SIZE = 8;
};