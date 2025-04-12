#include <Standards.hpp>
#include <network/Connection.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ToServerPacket.hpp>
#include <network/VarIntLong.hpp>
#include <network/CubaSocket.hpp>
#include <cstring> // for memcpy


// Must pass a valid network file descriptor
Connection::Connection(int fd) { 
    #ifdef DEBUG
        Console::getConsole().Entry("Creating connection");
    #endif
    this->file_d = fd;
    connected = true;
    setSocketBlocking(fd, false);
}

Connection::~Connection() = default;

void Connection::decode_packet(void* packetData, int packetID) {
    switch(myState) {
        case Handshake:
            if (packetID == 0x00) {
                // call server_Handshake();
                Console::getConsole().Error("In Handshake, 0x00 found.  Unhandled.");
            }
            else {
                Console::getConsole().Error("Unknown Packet ID in Handshake");
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
                Console::getConsole().Error("Unknown Packet ID in Status");
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
                    Console::getConsole().Error("Unknown Packet ID in Login");
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
                    Console::getConsole().Error("Unknown Packet ID in Config");
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
bool Connection::processIncPacket() {
    setSocketBlocking(file_d, true);
    int pLen = readVarIntFromSocket(this->getFD(), nullptr);
    bool has_data = false;
    int id = -1;
    void* buff;
    if (compress_threshold != -1 && pLen >= compress_threshold) {
        // decompress packet data and load buffer
        #warning "Need to handle packet decompression"
    }
    else {
        // handle packet normally
        int iLen = 0;
        id = readVarIntFromSocket(this->getFD(), &iLen);
        if ((pLen - iLen) > 0) {
            has_data = true;
            buff = malloc(sizeof(char) * (pLen - iLen));
            int error = Recieve(this->getFD(), buff, (pLen - iLen));
            if (error < 0) {
                Console::getConsole().Error("Error with recieving uncompressed packet data");
                setSocketBlocking(file_d, false);
                return false;
            }
        }
    }

    if (has_data) {
        decode_packet(buff, id);
        free(buff);
    }
    else {
        decode_packet(nullptr, id);
    }
    setSocketBlocking(file_d, false);
    return true;
}

int Connection::close() {
    int result = Closefd(file_d);
    if (0 == result) {
        #ifdef DEBUG
            Console::getConsole().Entry("Successfully closed connection fd: " + std::to_string(file_d));
        #endif
        connected = false;
        return 0;
    }
    #ifdef DEBUG
        Console::getConsole().Entry("Failed to close connection fd: " + std::to_string(file_d));
    #endif
    return -1;
}

// This parameter list could be reduced and calculated within the function
int Connection::addPending(Packet newPacket) {
    // TODO check to ensure packet is loaded with correct values
    pending.push(newPacket);
    return 0;
}

int Connection::countPending() {
    return pending.size();
}

// Must check if pending is non-empty before calling
// Will return a blank packet when attempting on an empty queue
Packet Connection::getPending() {
    Packet p;
    if (pending.size() > 0) {
        p = pending.front();
        pending.pop();
        return p;
    }
    Console::getConsole().Error("Attempted to get pending packet from empty queue.");
    return p;
}

int Connection::serializePacket(const Packet& packet, void*& buffer) {
    // Get the encoded bytes for packet length and ID
    std::vector<uint8_t> lengthBytes = packet.len.getBytes();
    std::vector<uint8_t> idBytes = packet.id.getBytes();
    
    // Calculate total buffer size
    int lengthSize = lengthBytes.size();
    int idSize = idBytes.size();
    int dataSize = packet.data.size();
    int totalSize = lengthSize + idSize + dataSize;
    
    // Allocate buffer (freed in sendPendingPackets)
    buffer = malloc(totalSize);
    if (!buffer) {
        Console::getConsole().Error("Failed to allocate memory for packet serialization");
        return -1;
    }
    
    // Copy data into the buffer
    uint8_t* ptr = static_cast<uint8_t*>(buffer);
    
    // Write length VarInt
    if (!lengthBytes.empty()) {
        std::memcpy(ptr, lengthBytes.data(), lengthSize);
        ptr += lengthSize;
    }
    
    // Write packet ID VarInt
    if (!idBytes.empty()) {
        std::memcpy(ptr, idBytes.data(), idSize);
        ptr += idSize;
    }
    
    // Write packet data if there is any
    if (!packet.data.empty()) {
        std::memcpy(ptr, packet.data.data(), dataSize);
    }
    
    return totalSize;
}

int Connection::sendPendingPackets() {
    int sentCount = 0;
    while (!pending.empty()) {
        Packet packet = pending.front();
        // Serialize the packet
        void* serializedData = nullptr;
        int dataSize = serializePacket(packet, serializedData);
        
        if (dataSize > 0 && serializedData != nullptr) {
            // Send the data
            int result = send(file_d, serializedData, dataSize, 0);
            // Malloc'd in serializePacket, so free it here
            free(serializedData);
            
            if (result < 0) {
                Console::getConsole().Error("Failed to send packet");
                return sentCount;
            }
            
            pending.pop();
            sentCount++;
        } else {
            Console::getConsole().Error("Failed to serialize packet");
            pending.pop();
        }
    }
    return sentCount;
}

void Connection::setState(Connection::Connection_State state) {
    myState = state;
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

void ConnectionList::removeConnection(Connection conn) {
    int fd = conn.getFD();
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        if (it->getFD() == fd) { // No const requirement here
            connections.erase(it); // Remove the connection
            count -= 1;
            Console::getConsole().Entry("Connection removed successfully: " + std::to_string(fd));
            return;
        }
    }
    Console::getConsole().Error("Connection not found by FD.");
}

void ConnectionList::closeAllConnections() {
    std::vector<Connection> failedCloses;
    int failures = 0;
    for (auto i = connections.begin(); i != connections.end(); ) { 
        int result = i->close();
        if (result == 0) {
            count -= 1;
            i = connections.erase(i);
        }
        else {
            failures += 1;
            failedCloses.push_back(*i);
            i++;
        }
    }
    if (count == 0) {
        Console::getConsole().Entry("All connections closed.");
    }
    else {
        count = failures;
        connections = failedCloses;
        Console::getConsole().Entry("Some connections failed to close when closing all.");
    }
}