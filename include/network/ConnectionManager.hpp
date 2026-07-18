#pragma once
#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <ThreadPool.hpp>
#include <atomic>
#include <mutex>

#define THREAD_COUNT 4

// Requires initialization
class ConnectionManager {
    public:
        static ConnectionManager& getInstance();
        void initialize();
        void close();
        // Snapshot of currently-alive connections. Safe to call from any thread.
        std::vector<std::shared_ptr<Connection>> getActiveConnections();
    private:
        ConnectionManager();
        ~ConnectionManager();
        void processConnection(std::shared_ptr<Connection> conn);
        void serverThreadLoop();
        std::unique_ptr<ServerSocket> _serverSocket;
        // weak_ptr: connections already manage their own lifetime through the
        // ThreadPool re-enqueue chain's shared_ptr captures. Owning them here
        // too would keep every connection alive forever.
        std::vector<std::weak_ptr<Connection>> _connections;
        std::mutex _connectionsMutex;
        std::thread _serverConThread;
        std::unique_ptr<ThreadPool> _conThreads;
        bool _initialized = false;
        std::atomic<bool> running{true};
};
