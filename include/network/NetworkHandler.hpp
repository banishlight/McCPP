#pragma once
#include <ThreadPool.hpp>
#include <thread>

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
        void close();
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