#pragma once
#include <Standards.hpp>
#include <vector>
// Will be using SOCK_STREAM sockets

bool setSocketBlocking(int fd, bool blocking);
int Bind(string ip, string port);
int Accept(int listen_fd);
int Listen(string ip, string port);
int Closefd(int fd);
int Recieve(int fd, void* buff, int size);
bool packetReady(int fd);
int checkErrno();
int readVarIntFromSocket(int fd, int* byte_count);
std::vector<UInt8> decompress(const void* compData, int size);