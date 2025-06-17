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
        bool isValid() const;
        void setState(ConnectionState state);
        void addPacket(std::shared_ptr<Outgoing_Packet> packet);
        void setUUID(std::vector<long> uuid);
        std::vector<long> getUUID() const;
        int getCompressionThreshold() const;
        void enableCompression();
        string getUsername() const;
        void setUsername(const string& username);
    private:
        void deserializePacket(std::vector<Byte> packet);
        std::vector<Byte> serializePacket(std::shared_ptr<Outgoing_Packet> packet);
        std::shared_ptr<Socket> _socket;
        std::vector<std::shared_ptr<Outgoing_Packet>> _sendQueue;
        ConnectionState _state = ConnectionState::Handshake;
        int _threshold = -1;
        bool _enableCompression = false;
        std::vector<long> _playerUUID;
        string _username;
        // TODO: std::shared_ptr<ActionProcessor> _actionProcessor;
        // TODO: string disconnectReason;
        // TODO: bool readyToDisconnect = false; // Maybe?
};