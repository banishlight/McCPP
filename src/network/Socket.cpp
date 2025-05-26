#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>

// Linux implementation
#ifdef LINUX
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <cstring>
#include <fcntl.h> // for non-blocking fd

int Socket::fetchVarInt() {
    int value = 0;
    int position = 0;
    Byte currentByte;
    
    do {
        // Read 1 byte
        int bytesRead = recv(_fd, &currentByte, 1, 0);
        if (bytesRead != 1) {
            throw std::runtime_error("Failed to read VarInt byte");
        }
        
        // Add the 7 bits to our value
        value |= (currentByte & 0x7F) << position;
        position += 7;
        
        if (position >= 32) {
            throw std::runtime_error("VarInt too big");
        }
        
    } while ((currentByte & 0x80) != 0);
    
    return value;
}

void Socket::sendData(const std::vector<Byte> data) {
    if (_fd < 0) {
        throw std::runtime_error("Invalid socket file descriptor");
    }
    
    if (data.empty()) {
        return; // Nothing to send
    }
    
    // Temporarily set to blocking for sending to ensure all data is sent
    bool wasBlocking = _blocking;
    if (!wasBlocking) {
        setBlocking(true);
    }
    
    size_t totalSent = 0;
    size_t dataSize = data.size();
    const Byte* dataPtr = data.data();
    
    try {
        while (totalSent < dataSize) {
            ssize_t sent = send(_fd, dataPtr + totalSent, dataSize - totalSent, 0);
            
            if (sent < 0) {
                throw std::runtime_error("Failed to send data: " + std::string(strerror(errno)));
            } else if (sent == 0) {
                throw std::runtime_error("Connection closed during send");
            }
            
            totalSent += sent;
        }
    } catch (...) {
        // Restore original blocking state before re-throwing
        if (!wasBlocking) {
            setBlocking(false);
        }
        throw;
    }
    
    // Restore original blocking state
    if (!wasBlocking) {
        setBlocking(false);
    }
}

// sock_fd is the file descriptor accepted from the server socket
Socket::Socket(int fd) {
    _fd = fd;
    if (isValid()) setBlocking(true);
    else throw std::runtime_error("Invalid socket file descriptor");
}

Socket::~Socket() {
    // If socket close fails, might cause a memory leak?
    close(_fd);
}

bool Socket::isValid() const {
    return _fd > 0;
}

std::vector<Byte> Socket::receivePacket() {
    if (_fd < 0) return std::vector<Byte>();
    if (!packetAvailable()) return std::vector<Byte>();

    // Deserialize packet size then
    // fetch the packet into vector
    int size = fetchVarInt();
    std::vector<Byte> buffer(size);
    // Unsure if I want to enforce blocking while receiving data
    ssize_t rec;
    if (_blocking == false) {
        setBlocking(true);
        rec = recv(_fd, buffer.data(), size, 0);
        setBlocking(false);
    }
    else {
        rec = recv(_fd, buffer.data(), size, 0);
    }
    if (rec < 0) {
        throw std::runtime_error("Failed to receive packet");
    } else if (rec == 0) {
        throw std::runtime_error("Connection closed");
    }
    if (rec != size) {
        throw std::runtime_error("Incomplete packet received");
    }
    return buffer;
}

bool Socket::packetAvailable() {
    char buff[8];  // Unsure if size is correct
    ssize_t rec = recv(_fd, buff, sizeof(buff), MSG_PEEK);
    return rec > 0;
}

void Socket::sendPacket(const Outgoing_Packet& packet) {
    if (_fd < 0) {
        throw std::runtime_error("Invalid socket file descriptor");
    }
    
    try {
        // Step 1: Serialize the packet data
        std::vector<Byte> packetData;
        int serializeResult = packet.serialize(packetData);
        
        if (serializeResult < 0) {
            throw std::runtime_error("Failed to serialize packet");
        }
        
        // Step 2: Get the packet ID as VarInt bytes
        std::vector<Byte> packetIdBytes = varIntSerialize(packet.getID());
        
        // Step 3: Calculate total packet size (ID + data)
        size_t totalPacketSize = packetIdBytes.size() + packetData.size();
        
        // Step 4: Get the packet length as VarInt bytes
        std::vector<Byte> lengthBytes = varIntSerialize(static_cast<int>(totalPacketSize));
        
        // Step 5: Combine all parts: [Length][PacketID][PacketData]
        std::vector<Byte> finalBuffer;
        
        // Pre-allocate space to avoid multiple reallocations
        finalBuffer.reserve(lengthBytes.size() + packetIdBytes.size() + packetData.size());
        
        // Add packet length
        finalBuffer.insert(finalBuffer.end(), lengthBytes.begin(), lengthBytes.end());
        
        // Add packet ID
        finalBuffer.insert(finalBuffer.end(), packetIdBytes.begin(), packetIdBytes.end());
        
        // Add packet data
        finalBuffer.insert(finalBuffer.end(), packetData.begin(), packetData.end());
        
        // Step 6: Send the complete packet
        sendData(finalBuffer);
        
        #ifdef DEBUG
        std::cout << "Sent packet ID: 0x" << std::hex << packet.getID() 
                  << ", size: " << std::dec << totalPacketSize << " bytes" << std::endl;
        #endif
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to send packet: ") + e.what());
    }
}

void Socket::setBlocking(bool block) {
    if (_fd < 0) return;
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1) return;
    flags = _blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(_fd, F_SETFL, flags);
    _blocking = block;
}

bool Socket::isBlocking() const {
    return _blocking;
}
#endif

// Windows implementation
#ifdef WINDOWS
#warning "Missing implementation for windows sockets"
Socket::Socket() {

}

Socket::~Socket() {
    
}

int Socket::getSocketFD() const {
    return 0;
}

void Socket::setSocketFD(int sock_fd) {

}

bool Socket::isValid() const {
    return false;
}
#endif
