#include "../../include/network/Connection.hpp"

class Connection {
    public:
        Connection::Connection() { 
            this->myState = Connection_State::Handshake;
        }

        Connection::~Connection() { 

        }

    private:
        void Connection::decode_packet(void* packet) {
            // determine packet ID
            int packetID;
            // get connection state
            
        }
};