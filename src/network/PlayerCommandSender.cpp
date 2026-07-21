#include <network/PlayerCommandSender.hpp>
#include <network/Connection.hpp>
#include <network/PacketUtils.hpp>
#include <network/packets/Play.hpp>
#include <OpsList.hpp>

PlayerCommandSender::PlayerCommandSender(std::shared_ptr<Connection> connection) {
    _connection = connection;
}

int PlayerCommandSender::getPermissionLevel() const {
    string uuidHex = uuidToHexString(_connection->getPlayer().getUUID());
    return OpsList::getInstance().getOpLevel(uuidHex);
}

string PlayerCommandSender::getName() const {
    return _connection->getPlayer().getUsername();
}

void PlayerCommandSender::sendMessage(const string& message) {
    int threshold = _connection->getCompressionThreshold();
    _connection->addPacket(std::make_shared<System_Chat_Message_p>(threshold, message));
}
