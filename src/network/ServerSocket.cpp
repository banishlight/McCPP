#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>


// Linux implementation
#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // close()

ServerSocket::ServerSocket(const string ip, const string port) {
    // Create a socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        // throw std::runtime_error("Failed to create socket");
    }

    // Set up the server address structure
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    // Bind the socket to the address
    if (bind(_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        // throw std::runtime_error("Failed to bind socket");
    }

    // Listen for incoming connections
    if (listen(_fd, SOMAXCONN) < 0) {
        // throw std::runtime_error("Failed to listen on socket");
    }
}

ServerSocket::~ServerSocket() {
    close(_fd);
}

Socket ServerSocket::Accept() {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        // Failed to accept a fd
    }
    return Socket(client_fd);
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