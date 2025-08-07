#include <Standards.hpp>
#include <network/Socket.hpp>
#include <Console.hpp>

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
            // If we return 0, then the socket must be closed client side so we should close it server side
            close();
            #ifdef DEBUG
                Console::getConsole().Error("Socket::fetchVarInt(): Failed to read VarInt byte: " + std::string(strerror(errno)));
                Console::getConsole().Error("Bytes read: " + std::to_string(bytesRead));
                Console::getConsole().Error("Socket::fetchVarInt(): Socket file descriptor: " + std::to_string(_fd));
            #endif
            return -1;
        }
        
        // Add the 7 bits to our value
        value |= (currentByte & 0x7F) << position;
        position += 7;
        
        if (position >= 32) {
            Console::getConsole().Error("Socket::fetchVarInt(): VarInt is too big, exceeds 32 bits.");
        }
        
    } while ((currentByte & 0x80) != 0);
    
    return value;
}

// fd is the file descriptor accepted from the server socket
Socket::Socket(int fd) {
    _fd = fd;
    // if (isValid()) setBlocking(true);
    // else Console::getConsole().Error("Socket::Socket(): Invalid socket file descriptor: " + std::to_string(fd));
}

Socket::~Socket() {
    // This was causing connections to close unintentionally
    // close(_fd);
}

bool Socket::isValid() const {
    if (_fd < 0) {
        #ifdef DEBUG
            Console::getConsole().Error("Socket::isValid(): Invalid socket file descriptor: " + std::to_string(_fd));
        #endif
        return false;
    }
    int result = fcntl(_fd, F_GETFD);
    if (result == -1) {
        Console::getConsole().Error("Socket::isValid(): Failed to get file descriptor flags: " + std::string(strerror(errno)));
        return false; 
    }
    #ifdef DEBUG
    Console::getConsole().Entry("Socket::isValid(): Socket file descriptor is valid: " + std::to_string(_fd));
    #endif
    return true;
}

// Assume: Check was already done, and something is available to be recieved
// Assume: Socket is valid
std::vector<Byte> Socket::receivePacket() {
    // Deserialize packet size
    int size = fetchVarInt();
    if (size < 0) {
        #ifdef DEBUG
            Console::getConsole().Error("Socket::receivePacket(): Invalid packet size: " + std::to_string(size));
        #endif
        return std::vector<Byte>();
    }
    // fetch the packet into vector
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
    #ifdef DEBUG
        if (rec < 0) {
            Console::getConsole().Error("Socket::receivePacket(): Failed to receive packet: " + std::string(strerror(errno)));
        } else if (rec == 0) {
            Console::getConsole().Error("Socket::receivePacket(): Connection closed by peer");
        }
        if (rec != size) {
            Console::getConsole().Error("Socket::receivePacket(): Incomplete packet received, expected " + std::to_string(size) + " bytes, got " + std::to_string(rec) + " bytes");
        }
    #endif
    return buffer;
}

bool Socket::packetAvailable() {
    #ifdef DEBUG
    Console::getConsole().Entry("Socket::packetAvailable(): Checking if packet is available on socket with file descriptor: " + std::to_string(_fd));
    #endif
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(_fd, &readfds);
    
    struct timeval timeout = {0, 0}; // Non-blocking check
    int result = select(_fd + 1, &readfds, nullptr, nullptr, &timeout);
    #ifdef DEBUG
    Console::getConsole().Entry("Socket::packetAvailable(): Received " + std::to_string(result) + " bytes from socket with file descriptor: " + std::to_string(_fd));
    #endif
    return result > 0;
}

void Socket::sendPacket(std::vector<Byte> data) {
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
                Console::getConsole().Error("Socket::sendData(): Failed to send data: " + std::string(strerror(errno)));
            } else if (sent == 0) {
                Console::getConsole().Error("Socket::sendData(): Connection closed during send");
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

void Socket::setBlocking(bool block) {
    if (_fd < 0) return;
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1) return;
    flags = block ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(_fd, F_SETFL, flags);
    _blocking = block;
}

bool Socket::isBlocking() const {
    return _blocking;
}

void Socket::close() {
    if (_fd < 0) return;
    if (::close(_fd) < 0) {
        Console::getConsole().Error("Socket::close(): Failed to close socket: " + std::string(strerror(errno)));
    }
    _fd = -1; // Mark as invalid
}

bool isValidFD(int fd) {
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
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
