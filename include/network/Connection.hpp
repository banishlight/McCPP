#pragma once
#include <unordered_map>
#include <vector>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>

class Connection {
    public:
        Connection(int fd);
        ~Connection();
        int getFD();
        bool isConnected();
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
        string ipaddress;
        int file_d = -1;
        void decode_packet(void* packet, int packetID);
        bool connected = false;
};

class ConnectionList {
    public:
        static ConnectionList& getList();
        void addConnection(Connection member);
        void setListenfd(int fd);
        int getListenfd();
        int close();
    private:
        int listen_fd = -1;
        int count = -1;
        std::vector<Connection> connections;
        ConnectionList();
        ~ConnectionList();
};
