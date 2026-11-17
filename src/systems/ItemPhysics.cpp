#include <systems/ItemPhysics.hpp>
#include <Chunk.hpp>
#include <BlockIds.hpp>
#include <FluidBlocks.hpp>
#include <CropBlocks.hpp>
#include <cmath>
#include <algorithm>

namespace ItemPhysics {

namespace {
    constexpr int MIN_Y = Chunk::WORLD_MIN_Y;
    constexpr int MAX_Y = Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1;

    // Tolerance vanilla uses when deciding whether boxes touch or a movement
    // is effectively zero.
    constexpr double COLLISION_EPSILON = 1.0e-7;

    // Fluid push strength per tick (vanilla, Overworld rates).
    constexpr double WATER_PUSH_SPEED = 0.014;
    constexpr double LAVA_PUSH_SPEED = 0.0023333333333333335;
    // A nearly stationary entity gets at least this much push.
    constexpr double MIN_PUSH_SPEED = 0.0045000000000000005;
    constexpr double NEAR_STATIONARY = 0.003;
    // Below this depth, each cell's flow is scaled down by the depth.
    constexpr double SHALLOW_FLOW_DEPTH = 0.4;

    // Item floats (instead of falling) once the fluid is deeper than this
    // over its bottom edge.
    constexpr double FLOAT_DEPTH = 0.1f;
    constexpr double BUOYANCY = 5.0e-4f;
    constexpr double BUOYANCY_MAX_RISE_SPEED = 0.06f;
    constexpr double WATER_DRAG = 0.99f;
    constexpr double LAVA_DRAG = 0.95f;

    // No per-block friction data yet (ice, slime etc.), so every surface uses
    // vanilla's default.
    constexpr float DEFAULT_BLOCK_FRICTION = 0.6f;

    // A source's own height (8/9) as vanilla rounds it, used when flow spills
    // over an edge into fluid one block lower.
    constexpr float SOURCE_HEIGHT = 0.8888889f;

    struct Vec3 {
        double x = 0.0, y = 0.0, z = 0.0;
        double length() const { return std::sqrt(x * x + y * y + z * z); }
        Vec3 scaled(double s) const { return {x * s, y * s, z * s}; }
        // Vectors this short normalize to zero, as in vanilla.
        Vec3 normalized() const {
            double len = length();
            return len < 1.0e-4 ? Vec3{} : Vec3{x / len, y / len, z / len};
        }
    };

    struct Box {
        double lo[3]; // x, y, z
        double hi[3];
    };

    Box BoxAt(const State& s) {
        return {{s.x - HALF_WIDTH, s.y, s.z - HALF_WIDTH}, {s.x + HALF_WIDTH, s.y + HEIGHT, s.z + HALF_WIDTH}};
    }

    int FloorInt(double v) { return static_cast<int>(std::floor(v)); }
    int CeilInt(double v) { return static_cast<int>(std::ceil(v)); }

    // Block lookup that tolerates any Y. Above the build limit is air; below
    // the bottom is treated as solid so items can't fall out of the world
    // (there's no void damage to clean them up).
    bool GetBlock(const BlockSource& blocks, int x, int y, int z, Int32& out) {
        if (y > MAX_Y) { out = AIR_BLOCK_STATE_ID; return true; }
        if (y < MIN_Y) { out = STONE_BLOCK_STATE_ID; return true; }
        return blocks.getBlock(x, y, z, out);
    }

    // Height of a block's collision box from its bottom; 0 means no
    // collision. Everything solid is treated as a full cube except farmland
    // (15/16 tall) -- there's no per-block shape data yet for slabs, stairs etc.
    double CollisionHeight(Int32 blockId) {
        if (blockId == AIR_BLOCK_STATE_ID) return 0.0;
        if (Fluid::typeOf(blockId) != Fluid::Type::None) return 0.0;
        if (Crop::isCrop(blockId)) return 0.0;
        if (Crop::isFarmland(blockId)) return 15.0 / 16.0;
        return 1.0;
    }

    bool BlocksMotion(Int32 blockId) { return CollisionHeight(blockId) > 0.0; }

    // ---- Collision ----

    // How far the box can actually travel along one axis (0=x, 1=y, 2=z)
    // before hitting a block, starting from the requested distance.
    double CollideAxis(const BlockSource& blocks, const Box& box, int axis, double distance) {
        if (std::abs(distance) < COLLISION_EPSILON) return 0.0;

        Box sweep = box;
        if (distance > 0) sweep.hi[axis] += distance; else sweep.lo[axis] += distance;

        const int x0 = FloorInt(sweep.lo[0] - COLLISION_EPSILON), x1 = FloorInt(sweep.hi[0] + COLLISION_EPSILON);
        const int y0 = FloorInt(sweep.lo[1] - COLLISION_EPSILON), y1 = FloorInt(sweep.hi[1] + COLLISION_EPSILON);
        const int z0 = FloorInt(sweep.lo[2] - COLLISION_EPSILON), z1 = FloorInt(sweep.hi[2] + COLLISION_EPSILON);

        for (int bx = x0; bx <= x1; bx++) {
            for (int by = y0; by <= y1; by++) {
                for (int bz = z0; bz <= z1; bz++) {
                    Int32 id;
                    // An unloaded neighbor counts as solid -- safer to stop
                    // at a chunk edge than to move into unknown space.
                    double height = GetBlock(blocks, bx, by, bz, id) ? CollisionHeight(id) : 1.0;
                    if (height <= 0.0) continue;
                    const double blo[3] = {double(bx), double(by), double(bz)};
                    const double bhi[3] = {bx + 1.0, by + height, bz + 1.0};

                    bool overlapsOtherAxes = true;
                    for (int a = 0; a < 3; a++) {
                        if (a == axis) continue;
                        if (!(box.lo[a] < bhi[a] - COLLISION_EPSILON && box.hi[a] > blo[a] + COLLISION_EPSILON)) {
                            overlapsOtherAxes = false;
                            break;
                        }
                    }
                    if (!overlapsOtherAxes) continue;

                    if (distance > 0 && box.hi[axis] <= blo[axis] + COLLISION_EPSILON) {
                        distance = std::min(distance, blo[axis] - box.hi[axis]);
                    } else if (distance < 0 && box.lo[axis] >= bhi[axis] - COLLISION_EPSILON) {
                        distance = std::max(distance, bhi[axis] - box.lo[axis]);
                    }
                }
            }
        }
        return std::abs(distance) < COLLISION_EPSILON ? 0.0 : distance;
    }

    struct MoveResult {
        bool collidedX, collidedY, collidedZ;
        bool onGround;
    };

    // Moves the state by its velocity with block collision, then applies
    // vanilla's post-collision rules: a blocked horizontal axis loses its
    // velocity, and any vertical collision stops vertical motion.
    MoveResult Move(const BlockSource& blocks, State& s) {
        const double dx = s.vx, dy = s.vy, dz = s.vz;
        Box box = BoxAt(s);

        // Vertical first, then the larger horizontal component, then the other.
        double ay = 0.0, ax = 0.0, az = 0.0;
        if (dy != 0.0) {
            ay = CollideAxis(blocks, box, 1, dy);
            box.lo[1] += ay; box.hi[1] += ay;
        }
        const bool zFirst = std::abs(dx) < std::abs(dz);
        if (zFirst && dz != 0.0) {
            az = CollideAxis(blocks, box, 2, dz);
            box.lo[2] += az; box.hi[2] += az;
        }
        if (dx != 0.0) {
            ax = CollideAxis(blocks, box, 0, dx);
            box.lo[0] += ax; box.hi[0] += ax;
        }
        if (!zFirst && dz != 0.0) {
            az = CollideAxis(blocks, box, 2, dz);
        }

        if (ax * ax + ay * ay + az * az > COLLISION_EPSILON) {
            s.x += ax;
            s.y += ay;
            s.z += az;
        }

        MoveResult result;
        result.collidedX = std::abs(dx - ax) >= 1.0e-5;
        result.collidedZ = std::abs(dz - az) >= 1.0e-5;
        result.collidedY = dy != ay;
        result.onGround = result.collidedY && dy < 0.0;

        if (result.collidedX) s.vx = 0.0;
        if (result.collidedZ) s.vz = 0.0;
        if (result.collidedY) s.vy = 0.0;
        return result;
    }

    // ---- Fluids ----

    // A fluid cell's own surface height in blocks (amount / 9; sources and
    // falling columns count as amount 8). 0 for anything not of this type.
    float OwnHeight(Int32 blockId, Fluid::Type type) {
        if (Fluid::typeOf(blockId) != type) return 0.0f;
        int amount = (Fluid::isSource(blockId) || Fluid::isFalling(blockId)) ? 8 : Fluid::amountOf(blockId);
        return static_cast<float>(amount) / 9.0f;
    }

    // Surface height as seen by entities: a full block when the same fluid
    // continues directly above, otherwise the cell's own height.
    float SurfaceHeight(const BlockSource& blocks, int x, int y, int z, Int32 blockId, Fluid::Type type) {
        Int32 above;
        if (GetBlock(blocks, x, y + 1, z, above) && Fluid::typeOf(above) == type) return 1.0f;
        return OwnHeight(blockId, type);
    }

    // Whether a block presents a solid face to the fluid (used for falling
    // fluid's downward pull along walls).
    bool IsSolidFace(const BlockSource& blocks, int x, int y, int z, Fluid::Type type) {
        Int32 id;
        if (!GetBlock(blocks, x, y, z, id)) return false;
        if (Fluid::typeOf(id) == type) return false;
        return BlocksMotion(id);
    }

    // Unit flow direction of one fluid cell, driven by height differences to
    // its four horizontal neighbors.
    Vec3 CellFlow(const BlockSource& blocks, int x, int y, int z, Int32 blockId, Fluid::Type type) {
        static constexpr int DIRS[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}}; // N, E, S, W
        const float ownHeight = OwnHeight(blockId, type);

        Vec3 flow;
        for (const auto& d : DIRS) {
            const int nx = x + d[0], nz = z + d[1];
            Int32 neighbor;
            if (!GetBlock(blocks, nx, y, nz, neighbor)) continue;
            Fluid::Type neighborType = Fluid::typeOf(neighbor);
            // A different fluid doesn't take part in this fluid's flow.
            if (neighborType != Fluid::Type::None && neighborType != type) continue;

            float neighborHeight = OwnHeight(neighbor, type);
            float diff = 0.0f;
            if (neighborHeight == 0.0f) {
                // Open space next to this cell only pulls flow toward it when
                // there's fluid one block down (spilling over an edge); a
                // solid block or open air over dry ground adds nothing.
                Int32 below;
                if (!BlocksMotion(neighbor) && GetBlock(blocks, nx, y - 1, nz, below)) {
                    float belowHeight = OwnHeight(below, type);
                    if (belowHeight > 0.0f) diff = ownHeight - (belowHeight - SOURCE_HEIGHT);
                }
            } else {
                diff = ownHeight - neighborHeight;
            }
            if (diff == 0.0f) continue;
            flow.x += static_cast<double>(static_cast<float>(d[0]) * diff);
            flow.z += static_cast<double>(static_cast<float>(d[1]) * diff);
        }

        // Falling fluid running down a wall also drags things downward.
        if (Fluid::isFalling(blockId)) {
            for (const auto& d : DIRS) {
                const int nx = x + d[0], nz = z + d[1];
                if (IsSolidFace(blocks, nx, y, nz, type) || IsSolidFace(blocks, nx, y + 1, nz, type)) {
                    flow = flow.normalized();
                    flow.y -= 6.0;
                    break;
                }
            }
        }
        return flow.normalized();
    }

    // Adds one fluid type's current to the velocity. Returns whether the
    // item touches that fluid at all; outDepth is how far the fluid surface
    // reaches above the item's bottom edge.
    bool ApplyFluidPush(const BlockSource& blocks, State& s, Fluid::Type type, double pushSpeed, double& outDepth) {
        outDepth = 0.0;
        Box box = BoxAt(s);
        for (int a = 0; a < 3; a++) { box.lo[a] += 0.001; box.hi[a] -= 0.001; }

        const int x0 = FloorInt(box.lo[0]), x1 = CeilInt(box.hi[0]);
        const int y0 = FloorInt(box.lo[1]), y1 = CeilInt(box.hi[1]);
        const int z0 = FloorInt(box.lo[2]), z1 = CeilInt(box.hi[2]);

        bool touching = false;
        double depth = 0.0;
        Vec3 sum;
        int contributing = 0;
        for (int x = x0; x < x1; x++) {
            for (int y = y0; y < y1; y++) {
                for (int z = z0; z < z1; z++) {
                    Int32 id;
                    // Vanilla skips fluid handling entirely next to unloaded chunks.
                    if (!GetBlock(blocks, x, y, z, id)) return false;
                    if (Fluid::typeOf(id) != type) continue;
                    double surface = static_cast<double>(static_cast<float>(y) + SurfaceHeight(blocks, x, y, z, id, type));
                    if (surface < box.lo[1]) continue;

                    touching = true;
                    depth = std::max(surface - box.lo[1], depth);
                    Vec3 flow = CellFlow(blocks, x, y, z, id, type);
                    if (depth < SHALLOW_FLOW_DEPTH) flow = flow.scaled(depth);
                    sum.x += flow.x; sum.y += flow.y; sum.z += flow.z;
                    contributing++;
                }
            }
        }

        if (sum.length() > 0.0) {
            Vec3 push = sum.scaled(1.0 / contributing).normalized().scaled(pushSpeed);
            if (std::abs(s.vx) < NEAR_STATIONARY && std::abs(s.vz) < NEAR_STATIONARY && push.length() < MIN_PUSH_SPEED) {
                push = push.normalized().scaled(MIN_PUSH_SPEED);
            }
            s.vx += push.x;
            s.vy += push.y;
            s.vz += push.z;
        }
        outDepth = depth;
        return touching;
    }
}

bool step(const BlockSource& blocks, const State& current, StepResult& out) {
    Int32 ownBlock;
    if (!blocks.getBlock(FloorInt(current.x), std::max(std::min(FloorInt(current.y), MAX_Y), MIN_Y),
                         FloorInt(current.z), ownBlock)) {
        return false;
    }

    State s = current;

    // 1. Currents (water first, then lava). Items get pushed twice per tick
    // in vanilla -- here, by the general per-entity update, and again at the
    // end of the tick (step 5) by the item's own update.
    double waterDepth, lavaDepth;
    bool inWater = ApplyFluidPush(blocks, s, Fluid::Type::Water, WATER_PUSH_SPEED, waterDepth);
    bool inLava = ApplyFluidPush(blocks, s, Fluid::Type::Lava, LAVA_PUSH_SPEED, lavaDepth);

    // Velocity before this tick's own forces, for the impulse check below.
    const double startVx = s.vx, startVy = s.vy, startVz = s.vz;

    // 2. Float when submerged enough, otherwise fall.
    if (inWater && waterDepth > FLOAT_DEPTH) {
        s.vx *= WATER_DRAG;
        s.vz *= WATER_DRAG;
        if (s.vy < BUOYANCY_MAX_RISE_SPEED) s.vy += BUOYANCY;
    } else if (inLava && lavaDepth > FLOAT_DEPTH) {
        s.vx *= LAVA_DRAG;
        s.vz *= LAVA_DRAG;
        if (s.vy < BUOYANCY_MAX_RISE_SPEED) s.vy += BUOYANCY;
    } else {
        s.vy -= GRAVITY;
    }

    // 3. Move with collision.
    MoveResult moved = Move(blocks, s);

    // 4. Friction -- ground friction is the supporting block's slipperiness
    // times the air value, computed in single precision as vanilla does.
    double horizontalFriction = AIR_FRICTION;
    if (moved.onGround) {
        horizontalFriction = static_cast<double>(DEFAULT_BLOCK_FRICTION * 0.98f);
    }
    s.vx *= horizontalFriction;
    s.vy *= VERTICAL_FRICTION;
    s.vz *= horizontalFriction;
    if (moved.onGround && s.vy < 0.0) s.vy *= -0.5;

    // 5. Second push, from the new position.
    double unusedDepth;
    bool inWaterAfter = ApplyFluidPush(blocks, s, Fluid::Type::Water, WATER_PUSH_SPEED, unusedDepth);
    bool inLavaAfter = ApplyFluidPush(blocks, s, Fluid::Type::Lava, LAVA_PUSH_SPEED, unusedDepth);

    const double jumpX = s.vx - startVx, jumpY = s.vy - startVy, jumpZ = s.vz - startVz;
    const bool velocityJumped = jumpX * jumpX + jumpY * jumpY + jumpZ * jumpZ > 0.01;

    out.next = s;
    out.onGround = moved.onGround;
    out.inFluid = inWaterAfter || inLavaAfter;
    out.hasImpulse = out.inFluid || velocityJumped;
    return true;
}

}
