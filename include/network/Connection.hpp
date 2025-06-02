#pragma once
#include <Standards.hpp>

// Forward declaration
class Socket;
class Outgoing_Packet;

class Connection {
    public:
        Connection(std::shared_ptr<Socket> socket);
        ~Connection();
        void receivePacket();
        void sendPackets();
        bool isValid();
        void setState(ConnectionState state);
    private:
        void deserializePacket(std::vector<Byte> packet);
        std::vector<Byte> serializePacket(std::shared_ptr<Outgoing_Packet> packet);
        std::shared_ptr<Socket> _socket;
        std::vector<std::shared_ptr<Outgoing_Packet>> _sendQueue;
        ConnectionState _state = ConnectionState::Handshake;
        bool _compression = false;
        int _threshold = -1;
        // TODO Hold reference to action processor
        // std::shared_ptr<ActionProcessor> _actionProcessor;
};
