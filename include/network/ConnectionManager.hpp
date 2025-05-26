#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <ThreadPool.hpp>

#define THREAD_COUNT 4

class ConnectionManager {
    public:
        ConnectionManager& getInstance();
        void initialize();

    private:
        ConnectionManager();
        ~ConnectionManager();
        void processConnection();
        void serverThreadLoop();
        ServerSocket _serverSocket;
        std::vector<Connection> _connections;
        std::thread _serverConThread;
        ThreadPool _conThreads;
};