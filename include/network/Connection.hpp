#pragma once
#include <Standards.hpp>

// Forward declaration
class Socket;
class Outgoing_Packet;

class Connection {
    public:
        Connection(std::unique_ptr<Socket> socket);
        ~Connection();
        void receivePackets();
        void sendPackets();
        bool isValid();
    private:
        std::unique_ptr<Socket> _socket;
        std::vector<std::shared_ptr<Outgoing_Packet>> _sendQueue;
        ConnectionState _state = ConnectionState::Handshake;
        bool _compression = false;
        int _threshold = -1;
};
