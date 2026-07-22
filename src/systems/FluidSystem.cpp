#include <systems/FluidSystem.hpp>
#include <FluidUpdateQueue.hpp>
#include <network/packets/Play.hpp>
#include <World.hpp>

void FluidSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    for (auto& [x, y, z] : FluidUpdateQueue::getInstance().tick()) {
        ResolveFluid(world, x, y, z);
    }
}

string FluidSystem::getName() const {
    return "Fluid";
}
