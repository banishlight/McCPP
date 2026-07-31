#pragma once
#include <Standards.hpp>
#include <chrono>

// Server-side record of a dropped item entity -- Spawn_Entity_p/Set_Entity_Metadata_p
// alone only tell clients what to render, nothing durable on the server side.
struct ItemEntity {
    int entityId;
    Int32 itemId;
    Int32 count;
    double x, y, z;
    double vx, vy, vz; // blocks/tick, driven by ItemPhysicsSystem
    int chunkX, chunkZ;
    std::chrono::steady_clock::time_point spawnTime;
    // How long after spawnTime this entity becomes pickup-able -- see
    // ITEM_PICKUP_DELAY_BREAK_SECONDS/ITEM_PICKUP_DELAY_TOSS_SECONDS
    // (ItemEntityManager.hpp).
    double pickupDelaySeconds;
    // Last position actually sent to clients -- distinct from x/y/z (the
    // internally-tracked current position, updated every tick) since
    // ItemPhysicsSystem's fluid-push corrections aren't broadcast every
    // tick. Needed as the baseline for Update_Entity_Position_p's delta
    // encoding. Initialized to the spawn position (Spawn_Entity_p already
    // told clients that much).
    double lastBroadcastX, lastBroadcastY, lastBroadcastZ;
};
