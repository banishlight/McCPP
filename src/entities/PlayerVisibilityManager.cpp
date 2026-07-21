#include <entities/PlayerVisibilityManager.hpp>
#include <network/Connection.hpp>
#include <network/ConnectionManager.hpp>
#include <network/packets/Play.hpp>
#include <Player.hpp>
#include <Chunk.hpp>
#include <cmath>

namespace {
    // minecraft:player entity-type registry ID, sourced from the vanilla data
    // generator report (server.jar --reports), same method used for
    // ITEM_ENTITY_TYPE_ID -- not guessed.
    const int PLAYER_ENTITY_TYPE_ID = 128;
}

PlayerVisibilityManager& PlayerVisibilityManager::getInstance() {
    static PlayerVisibilityManager instance;
    return instance;
}

PlayerVisibilityManager::VisibilityChange PlayerVisibilityManager::decideChange(int viewerEntityId, int targetEntityId, bool shouldBeVisible) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::set<int>& visibleSet = _visibleTo[viewerEntityId];
    bool currentlyVisible = visibleSet.count(targetEntityId) > 0;
    if (shouldBeVisible && !currentlyVisible) {
        visibleSet.insert(targetEntityId);
        return VisibilityChange::Spawn;
    }
    if (!shouldBeVisible && currentlyVisible) {
        visibleSet.erase(targetEntityId);
        return VisibilityChange::Despawn;
    }
    return VisibilityChange::None;
}

void PlayerVisibilityManager::refresh(std::shared_ptr<Connection> conn) {
    if (!conn) return;
    Player& player = conn->getPlayer();
    int playerEntityId = player.getEntityId();
    int playerChunkX = floorDiv16(static_cast<int>(std::floor(player.getX())));
    int playerChunkZ = floorDiv16(static_cast<int>(std::floor(player.getZ())));
    int threshold = conn->getCompressionThreshold();

    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& other : connections) {
        if (!other || other.get() == conn.get()) continue;
        if (other->getState() != ConnectionState::Play) continue;

        Player& otherPlayer = other->getPlayer();
        int otherEntityId = otherPlayer.getEntityId();
        int otherThreshold = other->getCompressionThreshold();
        int otherChunkX = floorDiv16(static_cast<int>(std::floor(otherPlayer.getX())));
        int otherChunkZ = floorDiv16(static_cast<int>(std::floor(otherPlayer.getZ())));

        bool connSeesOther = player.hasChunkLoaded(otherChunkX, otherChunkZ);
        VisibilityChange change = decideChange(playerEntityId, otherEntityId, connSeesOther);
        if (change == VisibilityChange::Spawn) {
            conn->addPacket(std::make_shared<Spawn_Entity_p>(threshold, otherEntityId, otherPlayer.getUUID(), PLAYER_ENTITY_TYPE_ID, otherPlayer.getX(), otherPlayer.getY(), otherPlayer.getZ()));
        } else if (change == VisibilityChange::Despawn) {
            conn->addPacket(std::make_shared<Remove_Entities_p>(threshold, otherEntityId));
        }

        bool otherSeesConn = otherPlayer.hasChunkLoaded(playerChunkX, playerChunkZ);
        VisibilityChange reverseChange = decideChange(otherEntityId, playerEntityId, otherSeesConn);
        if (reverseChange == VisibilityChange::Spawn) {
            other->addPacket(std::make_shared<Spawn_Entity_p>(otherThreshold, playerEntityId, player.getUUID(), PLAYER_ENTITY_TYPE_ID, player.getX(), player.getY(), player.getZ()));
        } else if (reverseChange == VisibilityChange::Despawn) {
            other->addPacket(std::make_shared<Remove_Entities_p>(otherThreshold, playerEntityId));
        }
    }
}

void PlayerVisibilityManager::handleDisconnect(std::shared_ptr<Connection> leaving) {
    if (!leaving) return;
    Player& player = leaving->getPlayer();
    int leavingEntityId = player.getEntityId();

    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& other : connections) {
        if (!other || other.get() == leaving.get()) continue;
        if (other->getState() != ConnectionState::Play) continue;
        int otherEntityId = other->getPlayer().getEntityId();

        bool wasVisible;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _visibleTo.find(otherEntityId);
            wasVisible = (it != _visibleTo.end() && it->second.erase(leavingEntityId) > 0);
        }
        if (wasVisible) {
            other->addPacket(std::make_shared<Remove_Entities_p>(other->getCompressionThreshold(), leavingEntityId));
        }
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _visibleTo.erase(leavingEntityId);
}
