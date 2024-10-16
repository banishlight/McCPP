#include <network/CubSock.hpp>
#include <Standards.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <cstring>
#include <Console.hpp>

CubSock::CubSock() {

}

CubSock::~CubSock() {

}

int CubSock::Bind(string ip, string port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    int addr_result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (addr_result != 0) { 
        Console::getConsole().Error("getaddrinfo failure");
        return -1; 
    } 
    int sock_result = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_result != 0) { 
        Console::getConsole().Error("socket creation failed");
        freeaddrinfo(res);
        return -1; 
    }
    if (bind(sock_result, res->ai_addr, res->ai_addrlen) < 0) {
        Console::getConsole().Error("Bind failed");
        freeaddrinfo(res);
        return -1;
    }
    return sock_result;
}

int CubSock::Accept(int listen_fd) {  // Sus implementation
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int new_sock = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_size);
    if (new_sock < 0) {
        Console::getConsole().Error("Accept failed");
        return -1;
    }
    return 0;
}

int CubSock::Listen(string ip, string port) {
    const int BACKLOG = 10;
    int sock_result = Bind(ip, port);
    if (sock_result < 0) {
        return -1; 
    }
    if (listen(sock_result, BACKLOG) < 0) {
        Console::getConsole().Error("Listen failed");
        return -1;
    }
    Console::getConsole().Entry("Listen successful on " + ip + ":" + port);
    return 0;
}

int CubSock::Connect(string ip, string port) {
    return 0;
}
