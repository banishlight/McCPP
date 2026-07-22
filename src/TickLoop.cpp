#include <TickLoop.hpp>
#include <systems/KeepAliveSystem.hpp>
#include <systems/ItemDespawnSystem.hpp>
#include <systems/ItemPhysicsSystem.hpp>
#include <systems/FallingBlockSystem.hpp>
#include <systems/AutosaveSystem.hpp>
#include <systems/ChunkUnloadSystem.hpp>
#include <systems/DayNightSystem.hpp>
#include <Console.hpp>
#include <chrono>

TickLoop& TickLoop::getInstance() {
    static TickLoop instance;
    return instance;
}

TickLoop::~TickLoop() {
    close();
}

void TickLoop::initialize() {
    if (_initialized) return;
    registerSystem(std::make_shared<KeepAliveSystem>());
    registerSystem(std::make_shared<ItemPhysicsSystem>());
    registerSystem(std::make_shared<FallingBlockSystem>());
    registerSystem(std::make_shared<ItemDespawnSystem>());
    registerSystem(std::make_shared<AutosaveSystem>());
    registerSystem(std::make_shared<ChunkUnloadSystem>());
    registerSystem(std::make_shared<DayNightSystem>());
    _tickThread = std::thread(&TickLoop::tickThreadLoop, this);
    _initialized = true;
}

void TickLoop::close() {
    if (!_initialized) return;
    running = false;
    if (_tickThread.joinable()) _tickThread.join();
    _initialized = false;
}

void TickLoop::registerSystem(std::shared_ptr<TickSystem> system) {
    _systems.push_back(std::move(system));
}

void TickLoop::tickThreadLoop() {
    auto nextTick = std::chrono::steady_clock::now();
    while (running) {
        nextTick += std::chrono::milliseconds(TICK_RATE_MS);
        _tickCount++;
        for (auto& system : _systems) {
            system->onTick(_tickCount);
        }
        #ifdef DEBUG
            if (_tickCount % 200 == 0) {
                Console::getConsole().Entry("TickLoop::tickThreadLoop(): tick " + std::to_string(_tickCount));
            }
        #endif
        auto now = std::chrono::steady_clock::now();
        if (nextTick > now) {
            std::this_thread::sleep_for(nextTick - now);
        } else {
            nextTick = now; // fell behind; don't spiral trying to catch up
        }
    }
}