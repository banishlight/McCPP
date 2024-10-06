#include <network/Connection.hpp>
#include <network/IPAddress.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ToServerPacket.hpp>

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
            if (packetID == 0x00) {
                // call server_Handshake();
            }
            else {
                Console::GetConsole().Error("Bad Packet ID in Handshake");
            }
            break;
        case Status:
            if () {
                // call server_Status_Request();
            }
            else if () {
                // call server_Ping_Request();
            }
            else {
                Console::GetConsole().Error("Bad Packet ID in Status");
            }
            break;
        case Login:
            // Decode one of four Login State packets
            break;
        case Config:
            break;
        case Play:
            // Function pointer array only used in play state for O(1) speed
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
        Console::getConsole().Error("No IP set.");
        return;// ERROR HERE, UNREACHABLE
    }
    this->connections.push_back(member);  
    this->count += 1;
}
