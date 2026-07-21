#pragma once
#include <Standards.hpp>
#include <Player.hpp>
#include <atomic>
#include <mutex>
#include <memory>

// Forward declaration
class Socket;
class Outgoing_Packet;
class Chunk;

class Connection : public std::enable_shared_from_this<Connection> {
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
        // Async chunk-generation completion delivery (see WorldWorkerPool /
        // World::getChunkAsync): addGeneratedChunk() is safe to call from any
        // thread (a world-worker thread completing generation). deliverGeneratedChunks()
        // must only be called from this connection's own processing thread --
        // it's where Player's loaded-chunk bookkeeping actually gets mutated,
        // keeping that state connection-thread-only even though generation itself
        // now happens elsewhere.
        void addGeneratedChunk(std::shared_ptr<Chunk> chunk);
        void deliverGeneratedChunks();
        // Per-login-attempt state for online-mode auth. Both are only ever
        // touched during the Login state, sequentially on this connection's
        // own processing thread, so no synchronization is needed -- mirrors
        // _threshold's unguarded style.
        void setVerifyToken(const std::vector<Byte>& token);
        const std::vector<Byte>& getVerifyToken() const;
        void setServerId(const std::string& serverId);
        const std::string& getServerId() const;
        // Queues a state-appropriate Disconnect packet with reason, then relies
        // on the client closing its own end on receipt (same as every existing
        // Disconnect_login_p use) rather than force-closing the socket from
        // here -- isValid() naturally goes false once that happens, on the
        // next receive attempt on this connection's own processing thread.
        void disconnect(const string& reason);
    private:
        void deserializePacket(std::vector<Byte> packet);
        std::vector<Byte> serializePacket(std::shared_ptr<Outgoing_Packet> packet);
        std::shared_ptr<Socket> _socket;
        std::vector<std::shared_ptr<Outgoing_Packet>> _sendQueue;
        std::mutex _sendQueueMutex;
        std::vector<std::shared_ptr<Chunk>> _pendingChunks;
        std::mutex _pendingChunksMutex;
        // atomic: read from the tick thread (getState(), Keep Alive) while
        // written from this connection's own worker thread (setState()).
        std::atomic<ConnectionState> _state{ConnectionState::Handshake};
        int _threshold = -1;
        bool _enableCompression = false;
        Player _player;
        std::vector<Byte> _verifyToken;
        std::string _serverId;
        // TODO: std::shared_ptr<ActionProcessor> _actionProcessor;
};