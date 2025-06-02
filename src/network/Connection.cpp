#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <network/PacketContext.hpp>
#include <Console.hpp>

Connection::Connection(std::shared_ptr<Socket> socket) {
    // _socket(std::move(socket));
    _socket = socket;
    _state = ConnectionState::Handshake;
}

Connection::~Connection() {

}

void Connection::deserializePacket(std::vector<Byte> packet) {
	int packetID = varIntDeserialize(packet);
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
    std::vector<Byte> serializedPacket;
    packet->serialize(serializedPacket);
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
        }
        else {
            Console::getConsole().Error("Connection::receivePacket(): Cannot receieve empty packets?.");
        }
    }
}

void Connection::sendPackets() {
    // Send all packets from queue
    for(auto& packet : _sendQueue) {
        std::vector<Byte> serializedPacket = serializePacket(packet);
        _socket->sendPacket(serializedPacket);
    }
    // Clear the queue
    _sendQueue.clear();
}

bool Connection::isValid() {
    return _socket->isValid();
}