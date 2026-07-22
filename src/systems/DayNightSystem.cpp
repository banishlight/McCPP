#include <systems/DayNightSystem.hpp>
#include <World.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>

void DayNightSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    world.advanceDayTime();

    if (tickCount % BROADCAST_INTERVAL_TICKS != 0) return;
    Int64 dayTime = world.getDayTime();
    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        conn->addPacket(std::make_shared<Update_Time_p>(conn->getCompressionThreshold(), dayTime));
    }
}

string DayNightSystem::getName() const {
    return "DayNight";
}
