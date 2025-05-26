#pragma once
#include <Standards.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>

class Connection {
    public:
        Connection(Socket socket);
        ~Connection();
        void receivePackets();
        void sendPackets();
        bool isValid();
    private:
        Socket _socket;
        std::vector<Outgoing_Packet> _sendQueue;
        ConnectionState _state = ConnectionState::Handshake;
        bool _compression = false;
        int _threshold = -1;
};
