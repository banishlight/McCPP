#include <Standards.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/Packet.hpp>


Connection::Connection(std::shared_ptr<Socket> socket) {
    // _socket(std::move(socket));
    _socket = socket;
    _state = ConnectionState::Handshake;
}

Connection::~Connection() {

}

int Connection::deserializePacket(std::vector<Byte> packet) {
	int packetID;

	return 0;
}

void Connection::receivePacket() {
    if (_socket->isValid()) {
        std::vector<Byte> packet = _socket->receivePacket();
        if (packet.size() > 0) {
            deserializePacket(packet);
        }
        else {
            // std::cerr << "Invalid socket. Cannot receive packets." << std::endl;
        }
    }
}

void Connection::sendPackets() {
    // Send all packets from queue
    // for(int i = 0; i < _sendQueue.size(); i++) {
    //     std::vector<Byte> bytes;
    //     _sendQueue[i]->serialize(bytes);
    //     _socket->sendPacket(bytes);
    // }
    for(auto& packet : _sendQueue) {
        _socket->sendPacket(*packet);
    }
    // Clear the queue
    _sendQueue.clear();
}

bool Connection::isValid() {
    return _socket->isValid();
}