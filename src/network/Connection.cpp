#include <network/Connection.hpp>
#include <network/IPAddress.hpp>

Connection::Connection() { 
    this->myState = Connection_State::Handshake;
}

Connection::~Connection() = default;

void Connection::decode_packet(void* packet) {
    // determine packet ID
    int packetID;
    // get connection state
    switch(myState) {
        case Handshake:
            break;
        case Status:
            break;
        case Login:
            break;
        case Play:
            break;
        case Closed:
            break;
    }
}

ConnectionList::ConnectionList() {

}

ConnectionList::~ConnectionList() {
    
}

ConnectionList& ConnectionList::getList() {
    static ConnectionList singleton;
    return singleton;
}