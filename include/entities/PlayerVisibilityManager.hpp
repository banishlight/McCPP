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
        // Tells every connection that currently has `mover` visible about
        // their new position/rotation -- called from the movement/rotation
        // packet handlers after Player's own state has already been updated.
        // oldX/Y/Z are the pre-update position, needed to compute the
        // position delta; rotation is always sent as an absolute angle (never
        // a delta), so no old yaw/pitch is needed.
        void broadcastMovement(std::shared_ptr<Connection> mover,
                               double oldX, double oldY, double oldZ,
                               bool positionChanged, bool rotationChanged, bool onGround);
        // Tells every connection that currently has `mover` visible about
        // their current sneaking/sprinting state (Set_Player_Pose_Metadata_p) --
        // called from Player_Command_p after Player's own state is updated.
        void broadcastPoseChange(std::shared_ptr<Connection> mover);
    private:
        PlayerVisibilityManager() = default;
        enum class VisibilityChange { None, Spawn, Despawn };
        // Locked check-and-update against _visibleTo[viewerEntityId]; the
        // caller sends the actual packet outside the lock based on the result.
        VisibilityChange decideChange(int viewerEntityId, int targetEntityId, bool shouldBeVisible);
        // Snapshot of every viewer entity ID that currently has targetEntityId
        // in its visible set -- shared by broadcastMovement and broadcastPoseChange.
        std::set<int> findViewersOf(int targetEntityId);

        std::mutex _mutex;
        std::unordered_map<int, std::set<int>> _visibleTo; // viewer entityId -> currently-spawned target entityIds
};
