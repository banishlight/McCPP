#pragma once
#include <mutex>
#include <unordered_map>
#include <set>
#include <memory>

class Connection;

// Tracks, for each player (by entity ID), which OTHER players' entities are
// currently spawned on their client -- proximity/chunk-based, separate from
// the server-wide tab list (BroadcastPlayerJoin/Player_Info_Update_p).
// Mirrors ItemEntityManager's singleton style.
class PlayerVisibilityManager {
    public:
        static PlayerVisibilityManager& getInstance();
        // Re-evaluates visibility both directions between conn's player and
        // every other active Play connection, sending Spawn_Entity_p/
        // Remove_Entities_p for whatever changed. Safe to call whenever a
        // player's own loaded-chunk set changes (chunk delivery) -- since both
        // directions are checked here, another player moving into or out of
        // conn's view is also caught the next time THAT player's own chunk
        // delivery triggers their refresh(), not just conn's.
        void refresh(std::shared_ptr<Connection> conn);
        // Despawns `leaving`'s player entity from every connection that
        // currently has them visible, and drops all bookkeeping for them
        // (both as a viewer and as a target).
        void handleDisconnect(std::shared_ptr<Connection> leaving);
    private:
        PlayerVisibilityManager() = default;
        enum class VisibilityChange { None, Spawn, Despawn };
        // Locked check-and-update against _visibleTo[viewerEntityId]; the
        // caller sends the actual packet outside the lock based on the result.
        VisibilityChange decideChange(int viewerEntityId, int targetEntityId, bool shouldBeVisible);

        std::mutex _mutex;
        std::unordered_map<int, std::set<int>> _visibleTo; // viewer entityId -> currently-spawned target entityIds
};
