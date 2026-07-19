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
};
