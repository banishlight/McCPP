#pragma once
#include <Standards.hpp>

// Will be using SOCK_STREAM sockets
int Bind(string ip, string port);
int Accept(int listen_fd);
int Listen(string ip, string port);
int Connect(string ip, string port);
int Poll();
int Closefd(int fd);