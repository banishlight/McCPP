#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <Console.hpp>

// Linux implementation
#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h> // AI_PASSIVE
#include <unistd.h> // close()
#include <cstring> // strerror

ServerSocket::ServerSocket(const string ip, const string port) {
    #ifdef DEBUG
    Console::getConsole().Entry("ServerSocket::ServerSocket(): Creating server socket on " + ip + ":" + port);
    #endif
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;     // Use my IP address
    
    int status = getaddrinfo(ip.empty() ? nullptr : ip.c_str(), port.c_str(), &hints, &res);
    if (status != 0) {
        Console::getConsole().Error("ServerSocket::ServerSocket(): Failed to get address info: " + std::string(gai_strerror(status)));
        return;
    }
    
    // Create socket using the address info
    _fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (_fd < 0) {
        Console::getConsole().Error("ServerSocket::ServerSocket(): Failed to create socket: " + std::string(strerror(errno)));
        freeaddrinfo(res);
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        Console::getConsole().Error("ServerSocket::ServerSocket(): Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
    }
    
    // Bind using the address from getaddrinfo (NOT your manual structure)
    if (bind(_fd, res->ai_addr, res->ai_addrlen) < 0) {
        Console::getConsole().Error("ServerSocket::ServerSocket(): Failed to bind socket: " + std::string(strerror(errno)));
        close(_fd);
        _fd = -1;
        freeaddrinfo(res);
        return;
    }
    
    // Clean up address info
    freeaddrinfo(res);
    
    // Listen for incoming connections
    if (listen(_fd, SOMAXCONN) < 0) {
        Console::getConsole().Error("ServerSocket::ServerSocket(): Failed to listen on socket: " + std::string(strerror(errno)));
        close(_fd);
        _fd = -1;
        return;
    }
    
    #ifdef DEBUG
    Console::getConsole().Entry("ServerSocket::ServerSocket(): Server socket created successfully");
    #endif
}

ServerSocket::~ServerSocket() {
    close(_fd);
}

Socket ServerSocket::Accept() {
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        Console::getConsole().Error("ServerSocket::Accept(): Failed to accept connection: " + std::string(strerror(errno)));
        return Socket(-1); // Return an invalid socket
    }
    return Socket(client_fd);
}

bool ServerSocket::isValid() {
    return _fd > 0;
}
#endif


// Windows implementation
#ifdef WINDOWS
#warning "Missing implementation for windows server sockets"

ServerSocket::ServerSocket(const string ip, const string port) : Socket(ip, port) {

}

ServerSocket::~ServerSocket() {

}

Socket ServerSocket::Accept() {

}
#endif