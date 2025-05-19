#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>


// Linux implementation
#ifdef LINUX
#include <sys/socket.h>

// sock_fd is the file descriptor accepted from the server socket
Socket::Socket(int sock_fd) {
    fd = sock_fd;
}

Socket::~Socket() {
    // If socket close fails, might cause a memory leak?
    close();
}

bool Socket::isValid() const {
    return fd > 0;
}

std::unique_ptr<Incoming_Packet> Socket::receivePacket(ConnectionState state) {
    if (fd < 0) return nullptr;
    if (!packetAvailable()) return nullptr;

    // Deserialize packet ID
    // Deserialize size
    // Then return the packet ptr?
}

bool Socket::packetAvailable() {
    char buff[8];  // Unsure if size is correct
    ssize_t rec = recv(fd, buff, sizeof(buff), MSG_PEEK);
    return rec > 0;
}

void Socket::sendPacket(const Outgoing_Packet* packet) {
    // Unsure of how I want to serialize then send the packet
}

void Socket::setBlocking(bool block) {
    if (fd < 0) return;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(fd, F_SETFL, flags);
    blocking = block;
}

bool Socket::isBlocking() const {
    return blocking;
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
