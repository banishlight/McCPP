#include <network/Connection.hpp>
#include <network/IPAddress.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ToServerPacket.hpp>
#include <network/VarIntLong.hpp>
#include <cstring>

Connection::Connection() { 

}

Connection::~Connection() = default;

void Connection::decode_packet(void* packet) {
    void* originalPacket = packet; // free later?
    VarInt packetLength = extractVarInt(&packet);
    Int32 packetID = extractVarInt(&packet).raw_ReadVarInt(); 
    switch(myState) {
        case Handshake:
            if (packetID == 0x00) {
                // call server_Handshake();
            }
            else {
                Console::getConsole().Error("Bad Packet ID in Handshake");
            }
            break;
        case Status:
            if (packetID == 0x00) {
                // call server_Status_Request();
            }
            else if (packetID == 0x01) {
                // call server_Ping_Request();
            }
            else {
                Console::getConsole().Error("Bad Packet ID in Status");
            }
            break;
        case Login:
            switch(packetID) {
                case 0x00:
                    // call server_Login_Start();
                    break;
                case 0x01:
                    // call server_Encryption_Response();
                    break;
                case 0x02:
                    // call server_Plugin_Response();
                    break;
                case 0x03:
                    // call server_Login_Acknowledged();
                    break;
                case 0x04:
                    // call server_Cookie_Reponse_login();
                    break;
                default:
                    Console::getConsole().Error("Bad Packet ID in Login");
            }
            break;
        case Config:
            switch(packetID) {
                case 0x00:
                    // call server_Client_Information();
                    break;
                case 0x01:
                    // call server_Cookie_Response();
                    break;
                case 0x02:
                    // call server_Plugin_Message();
                    break;
                case 0x03:
                    // call server_Acknowledge_Finish_Conifg();
                    break;
                case 0x04:
                    // call server_Keep_Alive();
                    break;
                case 0x05:
                    // call server_Pong();
                    break;
                case 0x06:
                    // call server_Resource_Pack_Reponse();
                    break;
                case 0x07:
                    // call server_Known_Packs();
                    break;
                default:
                    Console::getConsole().Error("Bad Packet ID in Config");
            }
            break;
        case Play:
            // Function pointer array only used in play state for O(1) speed
            break;
        case Closed:
            break;
    }
}

// Will need to be modified or other functions declared for variable sized types
// return value will need to be cast to the appropriate type and then freed
// WARNING, return value must be freed!
void* Connection::extractValue(void** packet, size_t size) {
    char** bytePtr = reinterpret_cast<char**>(packet);
    void* value = malloc(size);
    std::memcpy(value, *bytePtr, size);
    *bytePtr += size;
    return value;
}

// Wrong, PacketID's are stored as VarInt
int Connection::extractPacketID(void** packet) {
    char** bytePtr = reinterpret_cast<char**>(packet);
    int value = static_cast<int>(**bytePtr);
    *bytePtr += 1;
    return value;
}

VarInt Connection::extractVarInt(void** packet) {
    int value = 0;
    int position = 0;
    
}

VarLong Connection::extractVarLong(void** packet) {

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
    // Perform sorted insert here instead
}
