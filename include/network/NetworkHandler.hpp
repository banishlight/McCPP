#pragma once

class NetworkHandler {
    public:
        static NetworkHandler& getHandler();
    private:
        NetworkHandler();
        ~NetworkHandler();
};