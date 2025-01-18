#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <cstring>
#include <Console.hpp>
#include <network/CubaSocket.hpp>
#include <network/Connection.hpp>
#include <Standards.hpp> 
#include <fcntl.h> // for non-blocking fd
#include <errno.h> // for error checking
#include <zlib.h>

bool setSocketBlocking(int fd, bool blocking) {
    if (fd < 0) return false;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0);
}

int Bind(string ip, string port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    int addr_result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (addr_result != 0) { 
        Console::getConsole().Error("getaddrinfo failure");
        return -1; 
    } 
    int sock_result = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_result == -1) { 
        Console::getConsole().Error("socket creation failed");
        freeaddrinfo(res);
        return -1; 
    }
    if (bind(sock_result, res->ai_addr, res->ai_addrlen) < 0) {
        Console::getConsole().Error("Bind failed on ip: " + ip + ":" + port);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);
    return sock_result;
}

int Accept(int listen_fd) {  // Sus implementation
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int new_sock = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_size);
    if (new_sock < 0) {
        return -1;
    }
    return new_sock;
}

int Listen(string ip, string port) {
    const int BACKLOG = 10;
    int sock_result = Bind(ip, port);
    // set to non blocking fd
    if (!setSocketBlocking(sock_result, false)) {
        Console::getConsole().Error("set non-blocking fd failed");
        return -1;
    }
    if (sock_result < 0) {
        return -1; 
    }
    if (listen(sock_result, BACKLOG) < 0) {
        Console::getConsole().Error("Listen failed");
        // close(sock_result);
        return -1;
    }
    Console::getConsole().Entry("Listen successful on " + ip + ":" + port);
    return sock_result;
}

int Closefd(int fd) {
    return close(fd);
}

int Recieve(int fd, void* buff, int size) {
    return recv(fd, buff, size, 0);
}

// Checks if a packet is ready at the file descriptor
bool packetReady(int fd) {
    char buff[1024];
    ssize_t rec = recv(fd, buff, sizeof(buff), MSG_PEEK);
    if (rec <= 0) {
        return false;
    }
    return true;
}

int checkErrno() {
    #warning "Missing implementation"
    return 0;
}

// pass nullptr to byte_count to ignore byte counting
int readVarIntFromSocket(int fd, int* byte_count) {
	Int32 value = 0;
    int position = 0;
    uint8_t currentByte;
    if (byte_count != nullptr) {
        *byte_count = 0;
    }
    #ifdef DEBUG
        Console::getConsole().Entry("Starting VarInt Read");
    #endif
    while (true) {
        ssize_t bytesRead = recv(fd, &currentByte, 1, 0);
        #ifdef DEBUG
            Console::getConsole().Entry("Reading one byte for VarInt");
        #endif

        if (bytesRead == 0) {
            throw std::runtime_error("Connection closed while reading VarInt");
        } 
        else if (bytesRead < 0) {
            throw std::runtime_error("Error reading from socket");
        }

        // Accumulate the VarInt value
        value |= (currentByte & 0x7F) << position;
        if (byte_count != nullptr) {
            *byte_count += 1;
        }
        // Check if this byte is the last in the VarInt
        if ((currentByte & 0x80) == 0) {
            break;
        }

        position += 7;
        if (position >= 32) {
            throw std::runtime_error("VarInt is too big");
        }
    }
    return value;
}

std::vector<UInt8> decompress(const void* compData, size_t size) {
    #warning "Untested zlib decompression"
    std::vector<UInt8> decompData;
    z_stream stream{};
    stream.next_in = const_case<Bytef*> (compData);
    stream.avail_in = static_cast<uInt>(size);
    if (infateInit(&stream) != Z_OK) {
        Console::getConsole().Error("Failed to init zlib interface");
    }
    const size_t CHUNK_SIZE = 4096;
    UInt8 buffer[CHUNK_SIZE];
    int ret
    do {
        stream.next_out = buffer;
        stream.avail_out = CHUNK_SIZE;
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_MEM_ERROR || ret == Z_DATA_ERROR || ret == Z_STREAM_ERROR) {
            inflateEnd(&stream);
            throw std::runtime_error("Error during compression");
        }
        decompData.insert(decompData.end(), buffer, buffer + (CHUNK_SIZE - stream.avail_out));
    } while (ret != Z_STREAM_END);
    inflateEnd(&stream);
    return decompData;
}