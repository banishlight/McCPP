#pragma once
#include <ThreadPool.hpp>

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
        void close();
    private:
        NetworkHandler();
        ~NetworkHandler();
        ThreadPool* netThreads;
};