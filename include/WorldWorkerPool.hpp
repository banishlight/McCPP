#pragma once
#include <ThreadPool.hpp>
#include <functional>
#include <memory>

#define WORLD_WORKER_THREAD_COUNT 4

// The second of the project's two thread pools (the networking pool lives in
// ConnectionManager): shared by any CPU-bound, independent world-simulation
// work -- chunk generation today, lighting/pathing later. Never one pool per
// system.
class WorldWorkerPool {
    public:
        static WorldWorkerPool& getInstance();
        void initialize();
        void close();
        void submit(std::function<void()> task);
    private:
        WorldWorkerPool() = default;
        ~WorldWorkerPool();
        std::unique_ptr<ThreadPool> _pool;
        bool _initialized = false;
};