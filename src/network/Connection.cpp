#include <network/Connection.hpp>
#include <network/IPAddress.hpp>
#include <Properties.hpp>

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
    if (singleton.count == -1) {
        Properties& myProperties = Properties::getProperties();
        singleton.connections.reserve(myProperties.max_players);
        singleton.count = 0;
    }
    return singleton;
}

void ConnectionList::addConnection(Connection member) {
    if (this->count == -1) {
        return;// ERROR HERE, UNREACHABLE
    }
    this->connections.push_back(member);  
    this->count += 1;
}

int ConnectionList::setListenFD(int fd) {
    return 0;
}