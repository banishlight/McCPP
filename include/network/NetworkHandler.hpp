#pragma once

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
        void close();
    private:
        NetworkHandler();
        ~NetworkHandler();
};