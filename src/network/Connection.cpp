#include <network/Connection.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ToServerPacket.hpp>
#include <network/VarIntLong.hpp>
#include <network/CubSock.hpp>

Connection::Connection(int fd) { 
    this->file_d = fd;
}

Connection::~Connection() = default;

void Connection::decode_packet(void* packet, int packetID) {
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

int Connection::getFD() {
    return file_d;
}


ConnectionList::ConnectionList() {
    Properties& myProperties = Properties::getProperties();
    this->connections.reserve(myProperties.max_players);
    this->count = 0;
}

ConnectionList::~ConnectionList() {
    
}

ConnectionList& ConnectionList::getList() {
    static ConnectionList singleton;
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

void ConnectionList::setListenfd(int fd) {
    listen_fd = fd;
}

int ConnectionList::getListenfd() {
    return listen_fd;
}

int ConnectionList::close() {
    int result = 0;
    for (auto it = connections.begin(); it != connections.end(); it++) {
        result = Closefd(it->getFD());
        if (result != 0) {
            Console::getConsole().Error("Failed to close connection on fd: " + it->getFD());
            return -1;
        }
    }
    return 0;
}
