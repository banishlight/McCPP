#pragma once
#include <ThreadPool.hpp>
#include <network/Connection.hpp>
#include <thread>

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
        int initNetwork();
    private:
        NetworkHandler();
        ~NetworkHandler();
        void acceptConnectionsLoop();
        void processConnection(Connection conn);
        ThreadPool* netThreads;
        bool acceptConnections;
        std::thread acceptThread;
};