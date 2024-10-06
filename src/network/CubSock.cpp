#include <network/CubSock.hpp>
#include <Standards.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <cstring>

CubSock::CubSock() {

}

CubSock::~CubSock() {

}

int CubSock::Bind(string ip, string port) {
    struct addrinfo hints, *res;
    int addr_result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (addr_result != 0) { return -1; } // Error with IP
    int sock_result = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_result != 0) { return -1; }
    if (bind(sock_result, res->ai_addr, res->ai_addrlen) < 0) {
        return -1; // Bind failed
    }
    return 0;
}

int CubSock::Accept(string ip, string port) {
    return 0;
}

int CubSock::Listen(string ip, string port) {
// getaddrinfo();
// socket();
// bind();
// listen();
    struct addrinfo hints, *res;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    int addr_result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (addr_result != 0) { return -1; } // Error with IP
    int sock_result = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_result != 0) { return -1; }
    if (bind(sock_result, res->ai_addr, res->ai_addrlen) < 0) {
        // Bind failed
        return -1; 
    }
    if (listen(sock_result, 10) < 0) {
        // Listen failed
        return -1;
    }
    return 0;
}

int CubSock::Connect(string ip, string port) {
    return 0;
}
