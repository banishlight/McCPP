#include <systems/KeepAliveSystem.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>
#include <network/packets/Config.hpp>
#include <Console.hpp>

void KeepAliveSystem::onTick(Int64 tickCount) {
    if (tickCount % KEEP_ALIVE_INTERVAL_TICKS != 0) return;

    auto connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& conn : connections) {
        if (!conn) continue;
        int threshold = conn->getCompressionThreshold();
        switch (conn->getState()) {
            case ConnectionState::Play:
                conn->addPacket(std::make_shared<Clientbound_Keep_Alive_play_p>(threshold, tickCount));
                break;
            case ConnectionState::Config:
                conn->addPacket(std::make_shared<Clientbound_Keep_Alive_config_p>(threshold, tickCount));
                break;
            default:
                break; // Handshake/Status/Login/Closed: no Keep Alive in these states
        }
    }
}

string KeepAliveSystem::getName() const {
    return "KeepAlive";
}