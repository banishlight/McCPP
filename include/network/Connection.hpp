#pragma once
#include <Standards.hpp>
#include <Player.hpp>
#include <atomic>
#include <mutex>

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
        ConnectionState getState() const;
        void addPacket(std::shared_ptr<Outgoing_Packet> packet);
        int getCompressionThreshold() const;
        void enableCompression();
        void enableEncryption(const std::vector<Byte>& sharedSecret);
        Player& getPlayer();
    private:
        void deserializePacket(std::vector<Byte> packet);
        std::vector<Byte> serializePacket(std::shared_ptr<Outgoing_Packet> packet);
        std::shared_ptr<Socket> _socket;
        std::vector<std::shared_ptr<Outgoing_Packet>> _sendQueue;
        std::mutex _sendQueueMutex;
        // atomic: read from the tick thread (getState(), Keep Alive) while
        // written from this connection's own worker thread (setState()).
        std::atomic<ConnectionState> _state{ConnectionState::Handshake};
        int _threshold = -1;
        bool _enableCompression = false;
        Player _player;
        // TODO: std::shared_ptr<ActionProcessor> _actionProcessor;
        // TODO: string disconnectReason;
        // TODO: bool readyToDisconnect = false; // Maybe?
};