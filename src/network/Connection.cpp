#include <Standards.hpp>
#include <network/Connection.hpp>


Connection::Connection(Socket socket) {
    _socket = socket;
    _state = ConnectionState::Handshake;
}

Connection::~Connection() {

}

int Connection::deserializePacket(std::vector<Byte> packet) {
	int packetID;

	return 0;
}

void Connection::receivePackets() {
    if (_socket.isValid()) {
        std::vector<Byte> packet = _socket.receivePacket();
        if (packet.size() > 0) {
            deserializePacket(packet);
        }
        else {
            std::cerr << "Invalid socket. Cannot receive packets." << std::endl;
        }
    }
}

bool Connection::isValid() {
    return _socket.isValid();
}