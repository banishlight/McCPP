#pragma once
#include <unordered_map>
#include <vector>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>
#include <mutex>
#include <queue>

class Connection {
    public:
        Connection(int fd);
        ~Connection();
        int getFD();
        bool isConnected();
        bool processIncPacket();
        int close();
        int addPending(Packet newPacket);
        void setState(Connection::Connection_State state);
        int sendPendingPackets();
    
    private:
        Connection_State myState = Connection_State::Handshake;
        string ipaddress;
        int file_d = -1;
        bool connected = false;
        std::queue<Packet> pending;
        int compress_threshold = -1; // Enabled on non negative
        int countPending();
        Packet getPending();
        void decode_packet(void* packet, int packetID);
        int serializePacket(const Packet& packet, void*& buffer);
};
