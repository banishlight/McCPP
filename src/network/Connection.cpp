#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <network/PacketContext.hpp>
#include <network/Compression.hpp>
#include <Properties.hpp>
#include <Console.hpp>

Connection::Connection(std::shared_ptr<Socket> socket) {
    _socket = socket;
    _state = ConnectionState::Handshake;
    _playerUUID.reserve(2);
}

Connection::~Connection() {

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
    std::shared_ptr<Incoming_Packet> incomingPacket = registry.fetchIncomingPacket(_state, packetID);
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
        std::vector<Byte> packet = _socket->receivePacket();
        if (packet.size() > 0) {
            deserializePacket(packet);
        } else {
            Console::getConsole().Error("Connection::receivePacket(): Cannot receieve empty packets?.");
        }
    }
}

void Connection::sendPackets() {
    // Send all packets from queue
    for(auto& packet : _sendQueue) {
        std::vector<Byte> serializedPacket = serializePacket(packet);
        _socket->sendPacket(serializedPacket);
        // Set to true in Set_Compression_p, once sent, all future packets will be using compression threshold
        if (_enableCompression) {
            _threshold = Properties::getProperties().getCompressionThreshold();
            _enableCompression = false;
        }
    }
    // Clear the queue
    _sendQueue.clear();
}

bool Connection::isValid() const {
    return _socket->isValid();
}

void Connection::setState(ConnectionState state) {
    _state = state;
}

void Connection::addPacket(std::shared_ptr<Outgoing_Packet> packet) {
    if (packet == nullptr) {
        Console::getConsole().Error("Connection::addPacket(): Cannot add a null packet to the send queue.");
        return;
    }
    _sendQueue.push_back(packet);
}

void Connection::setPing(long ping) {
    _timestamp = ping;
}

long Connection::getPing() const {
    return _timestamp;
}

void Connection::setUUID(std::vector<long> uuid) {
    if (uuid.size() != 2) {
        Console::getConsole().Error("Connection::setUUID(): bad UUID vector size.");
        return;
    }
    _playerUUID = uuid;
}

std::vector<long> Connection::getUUID() const {
    return _playerUUID;
}

int Connection::getCompressionThreshold() const {
    return _threshold;
}

void Connection::enableCompression() {
    _enableCompression = true;
}