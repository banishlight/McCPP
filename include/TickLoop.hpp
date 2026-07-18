#pragma once
#include <Standards.hpp>
#include <TickSystem.hpp>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

// Requires initialization
//
// The server's single authoritative tick thread. Every registered TickSystem
// runs once per tick, in registration order, on this one thread -- this is
// deliberately not a worker pool. World-mutation ordering (vanilla-accurate
// redstone/hopper/AI tie-breaking order) must stay single-threaded; parallelism
// belongs only in a separate worker pool for genuinely independent work
// (chunk gen, lighting, pathfinding -- see the existing per-connection
// ThreadPool for the same idea applied to networking).
class TickLoop {
    public:
        static TickLoop& getInstance();
        void initialize();
        void close();
        // Only safe to call before initialize() starts the tick thread --
        // registration itself isn't synchronized, matching CommandRegistry's
        // registerCommand(). Don't add dynamic registration later without
        // revisiting this.
        void registerSystem(std::shared_ptr<TickSystem> system);
    private:
        TickLoop() = default;
        ~TickLoop();
        void tickThreadLoop();
        std::vector<std::shared_ptr<TickSystem>> _systems;
        std::thread _tickThread;
        std::atomic<bool> running{true};
        bool _initialized = false;
        Int64 _tickCount = 0;
        static constexpr int TICK_RATE_MS = 50; // 20 TPS, vanilla convention
};