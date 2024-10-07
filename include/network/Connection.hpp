#ifndef CONNECTION_H
#define CONNECTION_H
#include <string>
#include <unordered_map>
#include <vector>

class Connection {
    public:
        Connection();
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
        Connection_State myState;
        std::string ipaddress;
        int fd = -1;
        void decode_packet(void* packet);
        void* extractValue(void** packet, size_t size);
        int getPacketID(void** packet);
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

#endif