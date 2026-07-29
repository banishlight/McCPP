#include <systems/ItemPhysicsSystem.hpp>
#include <entities/ItemEntityManager.hpp>
#include <network/packets/Play.hpp>
#include <World.hpp>
#include <Chunk.hpp>
#include <BlockIds.hpp>
#include <FluidBlocks.hpp>
#include <cmath>
#include <algorithm>

namespace {
    // Height proxy for flow-gradient purposes, built from this project's own
    // level/distance abstractions (FluidBlocks.hpp) rather than reproducing
    // vanilla's fractional height formula -- sufficient for direction, which
    // is all this single-column, no-buoyancy item model needs. -1 means "not
    // part of this fluid's flow network" (a different fluid type, or a
    // genuinely solid block that blocks the edge).
    int FlowHeight(Int32 blockId, Fluid::Type matchType) {
        if (blockId == AIR_BLOCK_STATE_ID) return 0; // open edge -- pulls flow outward, matches vanilla
        Fluid::Type t = Fluid::typeOf(blockId);
        if (t != matchType) return -1;
        if (Fluid::isSource(blockId) || Fluid::isFalling(blockId)) return 8;
        // distanceOf already returns 8-level, i.e. HIGH near a source (level
        // close to 1) and LOW far away (level close to 7) -- exactly the
        // height ordering wanted here. Used directly, not re-inverted.
        return Fluid::distanceOf(blockId);
    }

    // Real vanilla sweeps entities (dropped items included) along with a
    // flowing current -- the actual mechanic behind "water auto farms"
    // collecting drops at one spot -- and the real client already simulates
    // this locally from the block state alone, with no server involvement.
    // Without a server-side equivalent, ItemEntityManager's authoritative
    // tracked position (what TryPickupNearbyItems' distance check and the
    // despawn sweep both use) stays wherever the item was originally dropped
    // even though the client visibly slides it along the current -- a real
    // reported bug (could see the item moved client-side, but had to walk to
    // where it USED to be to actually pick it up).
    bool ComputeFluidPush(World& world, int blockX, int blockY, int blockZ, Int32 fluidBlockId, double& outVx, double& outVz) {
        Fluid::Type type = Fluid::typeOf(fluidBlockId);
        if (type == Fluid::Type::None) return false;

        int baseHeight = (Fluid::isSource(fluidBlockId) || Fluid::isFalling(fluidBlockId))
                       ? 8 : Fluid::distanceOf(fluidBlockId);

        static constexpr int dx[4] = {1, -1, 0, 0};
        static constexpr int dz[4] = {0, 0, 1, -1};
        double vx = 0.0, vz = 0.0;
        for (int i = 0; i < 4; i++) {
            int nx = blockX + dx[i], nz = blockZ + dz[i];
            int nChunkX = floorDiv16(nx), nChunkZ = floorDiv16(nz);
            std::shared_ptr<Chunk> nChunk = world.getCachedChunk(nChunkX, nChunkZ);
            if (!nChunk) continue;
            Int32 neighborId = nChunk->getBlock(nx - nChunkX * 16, blockY, nz - nChunkZ * 16);
            int neighborHeight = FlowHeight(neighborId, type);
            if (neighborHeight < 0) continue; // solid, or a different fluid -- doesn't contribute
            int diff = baseHeight - neighborHeight;
            if (diff == 0) continue;
            vx += dx[i] * diff;
            vz += dz[i] * diff;
        }
        if (vx == 0.0 && vz == 0.0) return false; // symmetric surroundings (e.g. still water) -- no net push

        double length = std::sqrt(vx * vx + vz * vz);
        // Real vanilla constants, decompile-verified against the actual 1.21
        // client (Entity.updateFluidHeightAndDoFluidPushing's call sites):
        // water pushes at 0.014, lava at 0.0023333333333333335 in a
        // non-ultrawarm dimension (this project has no Nether, so that's the
        // only lava rate that applies -- the Nether rate is a separate 0.007).
        const double FLOW_SPEED = (type == Fluid::Type::Lava) ? 0.0023333333333333335 : 0.014;
        outVx = (vx / length) * FLOW_SPEED;
        outVz = (vz / length) * FLOW_SPEED;
        return true;
    }
}

void ItemPhysicsSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    ItemEntityManager& manager = ItemEntityManager::getInstance();

    for (const ItemEntity& entity : manager.snapshot()) {
        int chunkX = floorDiv16(static_cast<int>(std::floor(entity.x)));
        int chunkZ = floorDiv16(static_cast<int>(std::floor(entity.z)));
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) continue; // chunk not loaded -- leave it be, matches World::setBlock's own stance

        double candidateY = entity.y + entity.vy;
        int checkBlockY = static_cast<int>(std::floor(candidateY - 0.001));
        checkBlockY = std::max(checkBlockY, Chunk::WORLD_MIN_Y);
        checkBlockY = std::min(checkBlockY, Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1);

        int localX = static_cast<int>(std::floor(entity.x)) - chunkX * 16;
        int localZ = static_cast<int>(std::floor(entity.z)) - chunkZ * 16;
        Int32 blockBelow = chunk->getBlock(localX, checkBlockY, localZ);

        // An item resting on solid ground sits with its Y exactly at
        // groundBlockY+1 -- i.e. at the FLOOR of the block one layer above
        // its support, not one above that. If a block breaks/gets replaced
        // by fluid at the item's OWN layer (e.g. flowing water sweeps into
        // the exact spot where a just-dropped crop/plant item is already
        // resting on the ground below), that fluid is not "one block below"
        // the item at all -- it's occupying the same space the item's model
        // is actually sitting in. checkBlockY alone would never see it (a
        // real, reported bug: items settled before a current reached them
        // were never picked up by it at all, even after the fluid fully
        // stabilized). Check the item's own layer first and let it take
        // priority over whatever is supporting it from below.
        int ownBlockY = std::max(std::min(static_cast<int>(std::floor(entity.y)),
                                           Chunk::WORLD_MIN_Y + Chunk::WORLD_HEIGHT - 1), Chunk::WORLD_MIN_Y);
        Int32 blockAtOwnLayer = chunk->getBlock(localX, ownBlockY, localZ);
        Fluid::Type ownFluidType = Fluid::typeOf(blockAtOwnLayer);

        double newX = entity.x, newY = entity.y, newZ = entity.z;
        double newVx = entity.vx, newVy = entity.vy, newVz = entity.vz;
        bool onGround;

        Fluid::Type belowFluidType = Fluid::typeOf(blockBelow);
        if (ownFluidType != Fluid::Type::None || belowFluidType != Fluid::Type::None) {
            // Floating at the fluid's surface -- no buoyancy/depth model, just
            // rest here like solid ground, but pick up whatever push this
            // fluid tile's flow gradient produces (see ComputeFluidPush).
            //
            // Decompile-verified against the real client (ItemEntity.tick(),
            // obfuscated `cjh.l()`): the push is an ACCELERATION added to
            // existing velocity every tick, not a velocity replacement --
            // real vanilla then applies its own "underwater/lava movement"
            // drag (0.99 water / 0.95 lava) to the result, followed by the
            // same general per-tick friction every non-grounded entity gets
            // (a flat 0.98, since a floating item isn't resting on a solid
            // block in vanilla's own sense either). Recurrence at steady
            // state: v = (v + push) * drag * 0.98, which converges to a
            // terminal speed roughly 30x the raw push constant (e.g. water's
            // 0.014 push settles around ~0.46 blocks/tick, not 0.014) --
            // replacing velocity outright every tick (this project's first
            // attempt) was drastically slower than real vanilla's actual
            // current speed, which is what left the server's tracked
            // position perpetually trailing behind what the client rendered.
            int fluidBlockY = ownFluidType != Fluid::Type::None ? ownBlockY : checkBlockY;
            Int32 fluidBlockId = ownFluidType != Fluid::Type::None ? blockAtOwnLayer : blockBelow;
            Fluid::Type fluidType = Fluid::typeOf(fluidBlockId);
            newY = fluidBlockY + 1;
            double pushAx = 0.0, pushAz = 0.0;
            bool pushed = ComputeFluidPush(world, static_cast<int>(std::floor(entity.x)), fluidBlockY,
                                            static_cast<int>(std::floor(entity.z)), fluidBlockId, pushAx, pushAz);
            double vx = entity.vx + (pushed ? pushAx : 0.0);
            double vz = entity.vz + (pushed ? pushAz : 0.0);
            const double FLUID_MOVEMENT_DRAG = (fluidType == Fluid::Type::Lava) ? 0.95 : 0.99;
            const double GENERAL_FRICTION = 0.98; // applied every tick a non-grounded entity moves, real vanilla
            vx *= FLUID_MOVEMENT_DRAG * GENERAL_FRICTION;
            vz *= FLUID_MOVEMENT_DRAG * GENERAL_FRICTION;
            newVx = vx;
            newVz = vz;
            newVy = 0.0;
            newX = entity.x + newVx;
            newZ = entity.z + newVz;
            onGround = true;
        } else if (blockBelow != AIR_BLOCK_STATE_ID || checkBlockY <= Chunk::WORLD_MIN_Y) {
            // Solid support (or the world floor): rest exactly on top of it.
            newY = checkBlockY + 1;
            newVx = newVy = newVz = 0.0;
            onGround = true;
        } else {
            newY = candidateY;
            newX = entity.x + entity.vx;
            newZ = entity.z + entity.vz;
            newVy = entity.vy - GRAVITY;
            newVx = entity.vx * DRAG;
            newVz = entity.vz * DRAG;
            onGround = false;
        }

        if (newX == entity.x && newY == entity.y && newZ == entity.z &&
            newVx == entity.vx && newVy == entity.vy && newVz == entity.vz) {
            continue; // already settled, nothing changed -- no update, no packet
        }

        int newChunkX = floorDiv16(static_cast<int>(std::floor(newX)));
        int newChunkZ = floorDiv16(static_cast<int>(std::floor(newZ)));
        // Internal bookkeeping (pickup-radius/despawn accuracy) stays authoritative
        // every tick, independent of whether clients get a correction this tick.
        manager.updatePosition(entity.entityId, newX, newY, newZ, newVx, newVy, newVz, newChunkX, newChunkZ);

        // Broadcasting a position correction every tick fights the client's own
        // local physics simulation of this entity: only re-sync when velocity
        // changes by more than 0.1 units squared, trusting the client to
        // extrapolate the rest -- sending every tick caused visible stutter,
        // especially for the larger per-tick steps of a horizontal toss. This
        // applies to fluid push too, now that push itself is computed
        // correctly every tick (see the own-layer fluid check above): pickup
        // range and the despawn sweep both read `manager`'s internally tracked
        // position directly, which stays authoritative every tick regardless
        // of whether a broadcast happens, so nothing about correctness depends
        // on sending a correction here. A prior version of this forced a hard
        // re-sync on a fixed interval specifically while fluid-pushed, meant
        // to bound drift from an earlier (since-fixed) bug -- but with the
        // real bug gone, that forced correction just fought the client's own
        // otherwise-consistent prediction, causing a visible snap backward
        // every interval (a real, reported regression: "teleporting back
        // where they were every second interval"). Removed.
        double dvx = newVx - entity.vx, dvy = newVy - entity.vy, dvz = newVz - entity.vz;
        bool velocityDirty = (dvx * dvx + dvy * dvy + dvz * dvz) > 0.1;
        if (velocityDirty) {
            int entityId = entity.entityId;
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newX, newY, newZ, onGround](int threshold) {
                return std::make_shared<Teleport_Entity_p>(threshold, entityId, newX, newY, newZ, 0.0f, 0.0f, onGround);
            });
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newVx, newVy, newVz](int threshold) {
                return std::make_shared<Set_Entity_Velocity_p>(threshold, entityId, newVx, newVy, newVz);
            });
        }
    }
}

string ItemPhysicsSystem::getName() const {
    return "ItemPhysics";
}
