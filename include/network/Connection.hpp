#pragma once
#include <Standards.hpp>
#include <network/Socket.hpp>

class Connection {
    public:
        Connection(Socket socket);
        ~Connection();
        void receivePackets();
        bool isValid();
    private:
        Socket _socket;
        Connection_State _state = ConnectionState::Handshake;
        bool _compression = false;
        int _threshold = -1;
};
