#include <systems/CropGrowthSystem.hpp>
#include <CropGrowthQueue.hpp>
#include <network/packets/Play.hpp>
#include <World.hpp>

void CropGrowthSystem::onTick(Int64 tickCount) {
    World& world = World::getInstance();
    for (auto& [x, y, z] : CropGrowthQueue::getInstance().tick()) {
        ResolveCropGrowth(world, x, y, z);
    }
}

string CropGrowthSystem::getName() const {
    return "CropGrowth";
}
