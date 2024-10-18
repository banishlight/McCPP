#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <network/VarIntLong.hpp>

class Connection {
    public:
        Connection(int fd);
        ~Connection();
    private:
        enum Connection_State {
            Handshake,
            Status,
            Login,
            Config,
            Play,
            Closed
        };
        Connection_State myState = Connection_State::Handshake;
        std::string ipaddress;
        int file_d = -1;
        void decode_packet(void* packet, int packetID);
        void* extractValue(void** packet, size_t size);
        int extractPacketID(void** packet);
        VarInt extractVarInt(void** packet);
        VarLong extractVarLong(void** packet);
};

class ConnectionList {
    public:
        static ConnectionList& getList();
        void addConnection(Connection member);
    private:
        int count = -1; // -1 if uninitialized
        std::vector<Connection> connections;
        ConnectionList();
        ~ConnectionList();
};
