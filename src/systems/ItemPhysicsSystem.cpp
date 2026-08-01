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
    // Real vanilla item hitbox: 0.25 x 0.25 x 0.25 (EntityType.ITEM's fixed,
    // version-stable dimensions) -- only the horizontal footprint matters
    // here, since items already rest at an exact-integer Y (see onTick's
    // resting logic) and never straddle a Y boundary the way they can
    // straddle an X/Z one.
    constexpr double ITEM_HALF_WIDTH = 0.125;

    // Height proxy for flow-gradient purposes, built from this project's own
    // level/distance abstractions (FluidBlocks.hpp) rather than reproducing
    // vanilla's fractional height formula -- sufficient for direction, which
    // is all this single-Y-layer, no-buoyancy item model needs. -1 means
    // "not part of this fluid's flow network" (a different fluid type, or a
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

    // One fluid cell's own raw (unnormalized, unscaled) flow-direction
    // vector -- the weighted sum of unit vectors toward each of its 4
    // horizontal neighbors, by how much lower/higher that neighbor's height
    // is. This is the per-cell piece real vanilla's FluidState.getFlow
    // computes; an entity whose hitbox overlaps several cells averages
    // several of these together (see ComputeFluidPush) rather than trusting
    // only the single cell under its own reported position.
    bool ComputeCellFlow(World& world, int cellX, int cellZ, int blockY, Fluid::Type type, double& outVx, double& outVz) {
        int chunkX = floorDiv16(cellX), chunkZ = floorDiv16(cellZ);
        std::shared_ptr<Chunk> chunk = world.getCachedChunk(chunkX, chunkZ);
        if (!chunk) return false;
        Int32 cellId = chunk->getBlock(cellX - chunkX * 16, blockY, cellZ - chunkZ * 16);
        if (Fluid::typeOf(cellId) != type) return false; // this cell isn't (this) fluid -- doesn't contribute

        int baseHeight = (Fluid::isSource(cellId) || Fluid::isFalling(cellId)) ? 8 : Fluid::distanceOf(cellId);

        static constexpr int dx[4] = {1, -1, 0, 0};
        static constexpr int dz[4] = {0, 0, 1, -1};
        double vx = 0.0, vz = 0.0;
        for (int i = 0; i < 4; i++) {
            int nx = cellX + dx[i], nz = cellZ + dz[i];
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
        outVx = vx;
        outVz = vz;
        return true;
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
    //
    // Decompile-confirmed against the real client
    // (Entity.updateFluidHeightAndDoFluidPushing): it averages the flow
    // vector across every block cell the entity's actual bounding box
    // overlaps, not a single point under its reported position -- for a
    // 0.25-wide item that's normally one cell, but straddling a block
    // boundary can touch up to 4. Sampling only the item's own single column
    // (this project's first version) was a real, disclosed approximation
    // gap versus this real per-cell averaging.
    bool ComputeFluidPush(World& world, double entityX, double entityZ, int blockY, Int32 fluidBlockId,
                          double entityVx, double entityVz, double& outVx, double& outVz) {
        Fluid::Type type = Fluid::typeOf(fluidBlockId);
        if (type == Fluid::Type::None) return false;

        int minX = static_cast<int>(std::floor(entityX - ITEM_HALF_WIDTH));
        int maxX = static_cast<int>(std::floor(entityX + ITEM_HALF_WIDTH));
        int minZ = static_cast<int>(std::floor(entityZ - ITEM_HALF_WIDTH));
        int maxZ = static_cast<int>(std::floor(entityZ + ITEM_HALF_WIDTH));

        double sumVx = 0.0, sumVz = 0.0;
        int sampleCount = 0;
        for (int cx = minX; cx <= maxX; cx++) {
            for (int cz = minZ; cz <= maxZ; cz++) {
                double cvx, cvz;
                if (ComputeCellFlow(world, cx, cz, blockY, type, cvx, cvz)) {
                    sumVx += cvx;
                    sumVz += cvz;
                    sampleCount++;
                }
            }
        }
        if (sampleCount == 0) return false; // no overlapped cell is (this) fluid with a real gradient

        double vx = sumVx / sampleCount, vz = sumVz / sampleCount;
        double length = std::sqrt(vx * vx + vz * vz);
        if (length == 0.0) return false; // averaged out to nothing (e.g. still water) -- no net push

        // Real vanilla constants, decompile-verified against the actual 1.21
        // client (Entity.updateFluidHeightAndDoFluidPushing's call sites):
        // water pushes at 0.014, lava at 0.0023333333333333335 in a
        // non-ultrawarm dimension (this project has no Nether, so that's the
        // only lava rate that applies -- the Nether rate is a separate 0.007).
        const double FLOW_SPEED = (type == Fluid::Type::Lava) ? 0.0023333333333333335 : 0.014;
        double pushVx = (vx / length) * FLOW_SPEED;
        double pushVz = (vz / length) * FLOW_SPEED;

        // Real vanilla's minimum-speed floor: once the entity is already
        // nearly stationary and the computed push is weaker than 0.0045,
        // it's boosted up to exactly that magnitude in the same direction --
        // without this, lava's much weaker 0.0023 constant (the only case
        // this ever actually changes, since water's 0.014 already clears the
        // floor on its own) would leave a resting item barely moving instead
        // of visibly riding the current.
        const double MIN_PUSH_SPEED = 0.0045;
        const double NEAR_STATIONARY = 0.003;
        double pushLength = std::sqrt(pushVx * pushVx + pushVz * pushVz);
        if (std::abs(entityVx) < NEAR_STATIONARY && std::abs(entityVz) < NEAR_STATIONARY && pushLength < MIN_PUSH_SPEED) {
            pushVx = (pushVx / pushLength) * MIN_PUSH_SPEED;
            pushVz = (pushVz / pushLength) * MIN_PUSH_SPEED;
        }

        outVx = pushVx;
        outVz = pushVz;
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
            bool pushed = ComputeFluidPush(world, entity.x, entity.z, fluidBlockY, fluidBlockId,
                                            entity.vx, entity.vz, pushAx, pushAz);
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

        int entityId = entity.entityId;
        bool inFluid = (ownFluidType != Fluid::Type::None || belowFluidType != Fluid::Type::None);
        double dvx = newVx - entity.vx, dvy = newVy - entity.vy, dvz = newVz - entity.vz;
        bool velocityDirty = (dvx * dvx + dvy * dvy + dvz * dvz) > 0.1;

        if (inFluid) {
            // See FLUID_POSITION_BROADCAST_INTERVAL's comment (header): the
            // server's simplified push model can't stay bit-identical to the
            // real client's, so trajectories drift apart continuously. Rather
            // than wait for a big velocity change (which lets that drift
            // build up invisibly, then snaps the client backward all at once
            // -- a real, reported "rubberbanding" symptom), send small
            // position corrections on a fixed cadence, delta-encoded exactly
            // like PlayerVisibilityManager::broadcastMovement already does
            // for ordinary player movement (falling back to an absolute
            // Teleport_Entity_p only if a correction somehow exceeds the
            // delta format's +-8-block range).
            if (tickCount % FLUID_POSITION_BROADCAST_INTERVAL == 0) {
                double bdx = newX - entity.lastBroadcastX;
                double bdy = newY - entity.lastBroadcastY;
                double bdz = newZ - entity.lastBroadcastZ;
                if (bdx != 0.0 || bdy != 0.0 || bdz != 0.0) {
                    bool withinDeltaRange = std::abs(bdx) <= 7.99 && std::abs(bdy) <= 7.99 && std::abs(bdz) <= 7.99;
                    if (withinDeltaRange) {
                        Int16 deltaX = static_cast<Int16>(bdx * 4096.0);
                        Int16 deltaY = static_cast<Int16>(bdy * 4096.0);
                        Int16 deltaZ = static_cast<Int16>(bdz * 4096.0);
                        BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, deltaX, deltaY, deltaZ, onGround](int threshold) {
                            return std::make_shared<Update_Entity_Position_p>(threshold, entityId, deltaX, deltaY, deltaZ, onGround);
                        });
                    } else {
                        BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newX, newY, newZ, onGround](int threshold) {
                            return std::make_shared<Teleport_Entity_p>(threshold, entityId, newX, newY, newZ, 0.0f, 0.0f, onGround);
                        });
                    }
                    manager.markPositionBroadcast(entityId, newX, newY, newZ);
                }
            }
            // Velocity stays on the old "only when it meaningfully changes"
            // rule, decoupled from the position cadence above -- once a
            // current is established, the client extrapolates fine between
            // position corrections without a fresh velocity value every 4
            // ticks too.
            if (velocityDirty) {
                BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newVx, newVy, newVz](int threshold) {
                    return std::make_shared<Set_Entity_Velocity_p>(threshold, entityId, newVx, newVy, newVz);
                });
            }
        } else if (velocityDirty) {
            // Gravity/ground, unchanged: a real free-fall arc is one the
            // client can extrapolate identically from a single velocity
            // value (both sides run the same simple constant-gravity
            // formula), so rare corrections only on a real velocity change
            // are enough here -- this branch never runs the approximate
            // fluid model that motivated the above. Still updates the
            // fluid-delta baseline (markPositionBroadcast) so a later entry
            // into fluid push isn't computing its first delta against a
            // stale, long-out-of-date position.
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newX, newY, newZ, onGround](int threshold) {
                return std::make_shared<Teleport_Entity_p>(threshold, entityId, newX, newY, newZ, 0.0f, 0.0f, onGround);
            });
            BroadcastToChunkViewers(newChunkX, newChunkZ, [entityId, newVx, newVy, newVz](int threshold) {
                return std::make_shared<Set_Entity_Velocity_p>(threshold, entityId, newVx, newVy, newVz);
            });
            manager.markPositionBroadcast(entityId, newX, newY, newZ);
        }
    }
}

string ItemPhysicsSystem::getName() const {
    return "ItemPhysics";
}
