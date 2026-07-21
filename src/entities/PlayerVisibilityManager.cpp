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
            conn->addPacket(std::make_shared<Spawn_Entity_p>(threshold, otherEntityId, otherPlayer.getUUID(), PLAYER_ENTITY_TYPE_ID, otherPlayer.getX(), otherPlayer.getY(), otherPlayer.getZ(), otherPlayer.getYaw(), otherPlayer.getPitch(), otherPlayer.getYaw()));
            conn->addPacket(std::make_shared<Set_Player_Skin_Parts_Metadata_p>(threshold, otherEntityId, otherPlayer.getSkinParts()));
        } else if (change == VisibilityChange::Despawn) {
            conn->addPacket(std::make_shared<Remove_Entities_p>(threshold, otherEntityId));
        }

        bool otherSeesConn = otherPlayer.hasChunkLoaded(playerChunkX, playerChunkZ);
        VisibilityChange reverseChange = decideChange(otherEntityId, playerEntityId, otherSeesConn);
        if (reverseChange == VisibilityChange::Spawn) {
            other->addPacket(std::make_shared<Spawn_Entity_p>(otherThreshold, playerEntityId, player.getUUID(), PLAYER_ENTITY_TYPE_ID, player.getX(), player.getY(), player.getZ(), player.getYaw(), player.getPitch(), player.getYaw()));
            other->addPacket(std::make_shared<Set_Player_Skin_Parts_Metadata_p>(otherThreshold, playerEntityId, player.getSkinParts()));
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

void PlayerVisibilityManager::broadcastMovement(std::shared_ptr<Connection> mover,
                                                 double oldX, double oldY, double oldZ,
                                                 bool positionChanged, bool rotationChanged, bool onGround) {
    if (!mover) return;
    if (!positionChanged && !rotationChanged) return;

    Player& player = mover->getPlayer();
    int moverEntityId = player.getEntityId();
    double newX = player.getX(), newY = player.getY(), newZ = player.getZ();
    float newYaw = player.getYaw(), newPitch = player.getPitch();

    // Snapshot which viewers currently have this mover visible, then map back
    // to connections -- avoids holding _mutex while sending packets.
    std::set<int> viewerEntityIds;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& [viewerId, visibleSet] : _visibleTo) {
            if (visibleSet.count(moverEntityId)) viewerEntityIds.insert(viewerId);
        }
    }
    if (viewerEntityIds.empty()) return;

    double dx = newX - oldX, dy = newY - oldY, dz = newZ - oldZ;
    // Update Entity Position('s delta fields) can't represent a move beyond
    // 8 blocks in any axis -- fall back to an absolute Teleport Entity then.
    bool withinDeltaRange = std::abs(dx) <= 7.99 && std::abs(dy) <= 7.99 && std::abs(dz) <= 7.99;
    Int16 deltaX = static_cast<Int16>(dx * 4096.0);
    Int16 deltaY = static_cast<Int16>(dy * 4096.0);
    Int16 deltaZ = static_cast<Int16>(dz * 4096.0);

    std::vector<std::shared_ptr<Connection>> connections = ConnectionManager::getInstance().getActiveConnections();
    for (auto& viewerConn : connections) {
        if (!viewerConn || viewerConn.get() == mover.get()) continue;
        if (viewerConn->getState() != ConnectionState::Play) continue;
        if (!viewerEntityIds.count(viewerConn->getPlayer().getEntityId())) continue;

        int threshold = viewerConn->getCompressionThreshold();
        if (positionChanged && !withinDeltaRange) {
            viewerConn->addPacket(std::make_shared<Teleport_Entity_p>(threshold, moverEntityId, newX, newY, newZ, newYaw, newPitch, onGround));
            if (rotationChanged) {
                viewerConn->addPacket(std::make_shared<Set_Head_Rotation_p>(threshold, moverEntityId, newYaw));
            }
        } else if (positionChanged && rotationChanged) {
            viewerConn->addPacket(std::make_shared<Update_Entity_Position_and_Rotation_p>(threshold, moverEntityId, deltaX, deltaY, deltaZ, newYaw, newPitch, onGround));
            viewerConn->addPacket(std::make_shared<Set_Head_Rotation_p>(threshold, moverEntityId, newYaw));
        } else if (positionChanged) {
            viewerConn->addPacket(std::make_shared<Update_Entity_Position_p>(threshold, moverEntityId, deltaX, deltaY, deltaZ, onGround));
        } else { // rotationChanged only
            viewerConn->addPacket(std::make_shared<Update_Entity_Rotation_p>(threshold, moverEntityId, newYaw, newPitch, onGround));
            viewerConn->addPacket(std::make_shared<Set_Head_Rotation_p>(threshold, moverEntityId, newYaw));
        }
    }
}
