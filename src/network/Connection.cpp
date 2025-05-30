#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>
#include <network/PacketUtils.hpp>
#include <Console.hpp>

Connection::Connection(std::shared_ptr<Socket> socket) {
    // _socket(std::move(socket));
    _socket = socket;
    _state = ConnectionState::Handshake;
}

Connection::~Connection() {

}

int Connection::deserializePacket(std::vector<Byte> packet) {
	int packetID;
    packetID = varIntDeserialize(packet);
    Packet_Registry& registry = Packet_Registry::getInstance();
    std::shared_ptr<Incoming_Packet> incomingPacket = registry.fetchIncomingPacket(_state, packetID);
    incomingPacket->deserialize(packet, *this);
	return 0;
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
        _socket->sendPacket(*packet);
    }
    // Clear the queue
    _sendQueue.clear();
}

bool Connection::isValid() {
    return _socket->isValid();
}