#pragma once
#include <ThreadPool.hpp>

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
        void close();
        int initNetwork();
    private:
        NetworkHandler();
        ~NetworkHandler();
        ThreadPool* netThreads;
};