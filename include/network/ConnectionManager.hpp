#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <ThreadPool.hpp>

#define THREAD_COUNT 4

// Requires initialization
class ConnectionManager {
    public:
        static ConnectionManager& getInstance();
        void initialize();
        void close();
    private:
        ConnectionManager();
        ~ConnectionManager();
        void processConnection(std::shared_ptr<Connection> conn);
        void serverThreadLoop();
        std::unique_ptr<ServerSocket> _serverSocket;
        // std::vector<Connection> _connections;
        std::thread _serverConThread;
        std::unique_ptr<ThreadPool> _conThreads;
        bool _initialized = false;
        bool running = true;
};
