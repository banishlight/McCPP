#pragma once
#include <Standards.hpp>
#include <chrono>
#include <entities/EntityTracker.hpp>

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
    // What clients were last told about this item's movement (position
    // baseline for delta packets, last velocity, on-ground state) -- see
    // EntityTracker. Starts from what the spawn packet and any immediate
    // follow-up velocity packet carried.
    EntityTracker::State tracker;
};
