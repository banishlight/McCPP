#include <network/Connection.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ToServerPacket.hpp>
#include <network/VarIntLong.hpp>
#include <network/CubSock.hpp>

Connection::Connection(int fd) { 
    this->file_d = fd;
    connected = true;
}

Connection::~Connection() = default;

void Connection::decode_packet(void* packet, int packetID) {
    switch(myState) {
        case Handshake:
            if (packetID == 0x00) {
                // call server_Handshake();
                Console::getConsole().Error("In Handshake, 0x00 found.  Unhandled.");
            }
            else {
                Console::getConsole().Error("Bad Packet ID in Handshake");
            }
            break;
        case Status:
            if (packetID == 0x00) {
                // call server_Status_Request();
                Console::getConsole().Error("In Status, 0x00 found.  Unhandled.");
            }
            else if (packetID == 0x01) {
                // call server_Ping_Request();
                Console::getConsole().Error("In Status, 0x01 found.  Unhandled.");
            }
            else {
                Console::getConsole().Error("Bad Packet ID in Status");
            }
            break;
        case Login:
            switch(packetID) {
                case 0x00:
                    // call server_Login_Start();
                    Console::getConsole().Error("In Login, 0x00 found.  Unhandled.");
                    break;
                case 0x01:
                    // call server_Encryption_Response();
                    Console::getConsole().Error("In Login, 0x01 found.  Unhandled.");
                    break;
                case 0x02:
                    // call server_Plugin_Response();
                    Console::getConsole().Error("In Login, 0x02 found.  Unhandled.");
                    break;
                case 0x03:
                    // call server_Login_Acknowledged();
                    Console::getConsole().Error("In Login, 0x03 found.  Unhandled.");
                    break;
                case 0x04:
                    // call server_Cookie_Reponse_login();
                    Console::getConsole().Error("In Login, 0x04 found.  Unhandled.");
                    break;
                default:
                    Console::getConsole().Error("Bad Packet ID in Login");
            }
            break;
        case Config:
            switch(packetID) {
                case 0x00:
                    // call server_Client_Information();
                    Console::getConsole().Error("In Config, 0x00 found.  Unhandled.");
                    break;
                case 0x01:
                    // call server_Cookie_Response();
                    Console::getConsole().Error("In Config, 0x01 found.  Unhandled.");
                    break;
                case 0x02:
                    // call server_Plugin_Message();
                    Console::getConsole().Error("In Config, 0x02 found.  Unhandled.");
                    break;
                case 0x03:
                    // call server_Acknowledge_Finish_Conifg();
                    Console::getConsole().Error("In Config, 0x03 found.  Unhandled.");
                    break;
                case 0x04:
                    // call server_Keep_Alive();
                    Console::getConsole().Error("In Config, 0x04 found.  Unhandled.");
                    break;
                case 0x05:
                    // call server_Pong();
                    Console::getConsole().Error("In Config, 0x05 found.  Unhandled.");
                    break;
                case 0x06:
                    // call server_Resource_Pack_Reponse();
                    Console::getConsole().Error("In Config, 0x06 found.  Unhandled.");
                    break;
                case 0x07:
                    // call server_Known_Packs();
                    Console::getConsole().Error("In Config, 0x07 found.  Unhandled.");
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

bool Connection::isConnected() {
    return connected;
}

// Public call to grab packet from self fd and decode it
bool Connection::processPacket() {
    #warning "implementation unfinished"
    return false;
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
        Console::getConsole().Error("ConnectionList::addConnection() : No IP set.");
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

void ConnectionList::removeConnection(Connection conn) {
    int fd = conn.getFD();
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        if (it->getFD() == fd) { // No const requirement here
            connections.erase(it); // Remove the connection
            count -= 1;
            Console::getConsole().Entry("Connection removed successfully by FD.");
            return;
        }
    }
    Console::getConsole().Error("Connection not found by FD.");
}