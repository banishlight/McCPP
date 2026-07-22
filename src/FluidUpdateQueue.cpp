#include <FluidUpdateQueue.hpp>

FluidUpdateQueue& FluidUpdateQueue::getInstance() {
    static FluidUpdateQueue instance;
    return instance;
}

void FluidUpdateQueue::schedule(int x, int y, int z, int delayTicks) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto key = std::make_tuple(x, y, z);
    auto it = _countdowns.find(key);
    if (it == _countdowns.end() || it->second > delayTicks) {
        _countdowns[key] = delayTicks;
    }
}

std::vector<std::tuple<int,int,int>> FluidUpdateQueue::tick() {
    std::vector<std::tuple<int,int,int>> due;
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _countdowns.begin(); it != _countdowns.end(); ) {
        if (--(it->second) <= 0) {
            due.push_back(it->first);
            it = _countdowns.erase(it);
        } else {
            ++it;
        }
    }
    return due;
}
