#pragma once
#include <Standards.hpp>

// Server-side record of an in-flight falling block (sand/gravel that lost its
// support) -- mirrors ItemEntity's role: Spawn_Entity_p alone only tells
// clients what to render, nothing durable server-side. Only ever moves
// straight down (matches vanilla; nothing in this project pushes a falling
// block sideways), so no vx/vz is tracked.
struct FallingBlockEntity {
    int entityId;
    Int32 blockStateId;
    double x, y, z;
    double vy; // blocks/tick, driven by FallingBlockSystem
    int chunkX, chunkZ;
};
