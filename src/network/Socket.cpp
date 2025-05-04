#include <Standards.hpp>
#include <network/Socket.hpp>


// Linux implementation
#ifdef LINUX
#include <sys/socket.h>

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

void Socket::close() {

}

bool Socket::isClosed() const {
    return false;
}

#endif

// Windows implementation
#ifdef WINDOWS

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

void Socket::close() {

}

bool Socket::isClosed() const {
    return false;
}

#endif