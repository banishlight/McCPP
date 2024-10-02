#ifndef CONNECTION_H
#define CONNECTION_H
#include <string>
#include <unordered_map>

class Connection {
    public:
        Connection();
        ~Connection();
    private:
        enum Connection_State {
            Handshake,
            Status,
            Login,
            Play,
            Closed
        };
        Connection_State myState;
        std::string ipaddress;
        void decode_packet(void* packet);
};

class ConnectionList {
    public:
        static ConnectionList& getList();
    private:
        ConnectionList();
        ~ConnectionList();
};

#endif