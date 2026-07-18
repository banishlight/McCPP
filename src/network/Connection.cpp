#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <network/PacketContext.hpp>
#include <network/Compression.hpp>
#include <network/packets/Play.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <Chunk.hpp>
#include <cstdlib>

Connection::Connection(std::shared_ptr<Socket> socket) {
    _socket = socket;
    _state = ConnectionState::Handshake;
    #ifdef DEBUG
    Console::getConsole().Entry("Connection::Connection(): Created new connection");
    #endif
}

Connection::~Connection() {
    #ifdef DEBUG
    Console::getConsole().Entry("Connection::~Connection(): Closing Connection");
    #endif
}

void Connection::deserializePacket(std::vector<Byte> packet) {
    // TODO: Refactor compression handling
    if (_threshold != -1) { // Compression enabled
        int length = varIntDeserialize(packet);
        // Remove length from packet
        packet.erase(packet.begin(), packet.begin() + getVarIntSize(length));
        if (packet.size() >= static_cast<unsigned long>(_threshold)) {
            packet = decompressData(packet);
        }
        else {
            // VarInt must be zero to indicate no compression
            if (length != 0) {
                return; // Throw away the packet
            }
        }
    }
    int packetID = varIntDeserialize(packet);
    // Remove packet ID from packet
    packet.erase(packet.begin(), packet.begin() + getVarIntSize(packetID));
    Packet_Registry& registry = Packet_Registry::getInstance();
    std::shared_ptr<Incoming_Packet> incomingPacket = registry.fetchIncomingPacket(_state.load(), packetID);
    if (!incomingPacket) {
        #ifdef DEBUG
            Console::getConsole().Entry("Connection::deserializePacket(): No handler for packet ID " + std::to_string(packetID) + " in state " + std::to_string(_state.load()) + "; ignoring.");
        #endif
        return;
    }
    PacketContext cont(*this);
    // TODO set action processor for context if needed
    incomingPacket->deserialize(packet, cont);
	return;
}

std::vector<Byte> Connection::serializePacket(std::shared_ptr<Outgoing_Packet> packet) {
    if (packet == nullptr) {
        // If packet is null, something has gone very wrong.
        Console::getConsole().Error("Connection::serializePacket(): Cannot serialize a null packet.");
        return std::vector<Byte>();
    }
    // Serialize the packet and add it to the send queue
    std::vector<Byte> serializedPacket = packet->serialize();
    if (serializedPacket.empty()) {
        Console::getConsole().Error("Connection::serializePacket(): Serialized packet is empty.");
        return std::vector<Byte>();
    }
    return serializedPacket;
}

void Connection::receivePacket() {
    if (_socket->isValid()) {
        if (!_socket->packetAvailable())  {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return;
        }
        #ifdef DEBUG
            Console::getConsole().Entry("Connection::receivePacket(): Packet ready to be received.");
        #endif
        std::vector<Byte> packet = _socket->receivePacket();
        if (packet.size() > 0) {
            deserializePacket(packet);
        } else {
            #ifdef DEBUG
                Console::getConsole().Error("Connection::receivePacket(): Packet Receive error.");
            #endif
        }
    }
    else {
        Console::getConsole().Error("Connection::receivePacket(): Socket is not valid, cannot receive packets.");
    }
}

void Connection::sendPackets() {
    // Swap the queue out under the lock, then serialize/send the local copy
    // lock-free so addPacket() (called from other threads, e.g. the tick
    // thread) never blocks on a socket send().
    std::vector<std::shared_ptr<Outgoing_Packet>> toSend;
    {
        std::lock_guard<std::mutex> lock(_sendQueueMutex);
        toSend.swap(_sendQueue);
    }
    for(auto& packet : toSend) {
        std::vector<Byte> serializedPacket = serializePacket(packet);
        _socket->sendPacket(serializedPacket);
        // Set to true in Set_Compression_p, once sent, all future packets will be using compression threshold
        if (_enableCompression) {
            _threshold = Properties::getProperties().getCompressionThreshold();
            _enableCompression = false;
        }
    }
}

bool Connection::isValid() const {
    // TODO: Check Connection class is valid here
    return _socket->isValid();
}

void Connection::setState(ConnectionState state) {
    _state = state;
}

ConnectionState Connection::getState() const {
    return _state.load();
}

void Connection::addPacket(std::shared_ptr<Outgoing_Packet> packet) {
    if (packet == nullptr) {
        Console::getConsole().Error("Connection::addPacket(): Cannot add a null packet to the send queue.");
        return;
    }
    std::lock_guard<std::mutex> lock(_sendQueueMutex);
    _sendQueue.push_back(packet);
}

int Connection::getCompressionThreshold() const {
    return _threshold;
}

void Connection::enableCompression() {
    _enableCompression = true;
}

void Connection::enableEncryption(const std::vector<Byte>& sharedSecret) {
    _socket->enableEncryption(sharedSecret);
}

Player& Connection::getPlayer() {
    return _player;
}

void Connection::addGeneratedChunk(std::shared_ptr<Chunk> chunk) {
    std::lock_guard<std::mutex> lock(_pendingChunksMutex);
    _pendingChunks.push_back(chunk);
}

void Connection::deliverGeneratedChunks() {
    std::vector<std::shared_ptr<Chunk>> ready;
    {
        std::lock_guard<std::mutex> lock(_pendingChunksMutex);
        ready.swap(_pendingChunks);
    }
    if (ready.empty()) return;
    int viewDistance = _player.getViewDistance();
    int centerX = _player.getCenterChunkX();
    int centerZ = _player.getCenterChunkZ();
    int threshold = getCompressionThreshold();
    for (auto& chunk : ready) {
        int cx = chunk->getChunkX();
        int cz = chunk->getChunkZ();
        // Already delivered (duplicate in-flight request) or no longer within
        // view (the player moved away again before generation finished) --
        // either way, silently drop it. Never marked loaded, so no unload is needed.
        if (_player.hasChunkLoaded(cx, cz)) continue;
        if (std::abs(cx - centerX) > viewDistance || std::abs(cz - centerZ) > viewDistance) continue;
        _player.markChunkLoaded(cx, cz);
        addPacket(std::make_shared<Chunk_Data_p>(threshold, chunk));
    }
}