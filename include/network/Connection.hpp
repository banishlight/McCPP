#ifndef CONNECTION_H
#define CONNECTION_H

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
        int ipaddress;
        void decode_packet(void* packet);
};



#endif