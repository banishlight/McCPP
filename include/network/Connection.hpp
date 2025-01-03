#pragma once
#include <unordered_map>
#include <vector>
#include <Standards.hpp>
#include <network/VarIntLong.hpp>
#include <mutex>
#include <queue>

class Connection {
    public:
        enum Connection_State {
            Handshake,
            Status,
            Login,
            Config,
            Play,
            Closed
        };
        Connection(int fd);
        ~Connection();
        int getFD();
        bool isConnected();
        bool processIncPacket();
        int close();
        int addPending(int size, int id, int dsize, std::vector<Byte> data);
        int countPending();
        Packet getPending();
        void setState(Connection::Connection_State state);
    private:
        
        Connection_State myState = Connection_State::Handshake;
        string ipaddress;
        int file_d = -1;
        void decode_packet(void* packet, int packetID);
        bool connected = false;
        std::queue<Packet> pending;
};

class ConnectionList {
    public:
        static ConnectionList& getList();
        void addConnection(Connection member);
        void setListenfd(int fd);
        int getListenfd();
        void removeConnection(Connection conn);
        void closeAllConnections();
    private:
        int listen_fd = -1;
        int count = -1;
        std::vector<Connection> connections;
        ConnectionList();
        ~ConnectionList();
};
