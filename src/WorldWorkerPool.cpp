#include <WorldWorkerPool.hpp>

WorldWorkerPool& WorldWorkerPool::getInstance() {
    static WorldWorkerPool instance;
    return instance;
}

WorldWorkerPool::~WorldWorkerPool() {
    close();
}

void WorldWorkerPool::initialize() {
    if (_initialized) return;
    _pool = std::make_unique<ThreadPool>(WORLD_WORKER_THREAD_COUNT);
    _initialized = true;
}

void WorldWorkerPool::close() {
    if (!_initialized) return;
    // reset() (not a manual destructor call) so this isn't destroyed a second
    // time when the unique_ptr itself is torn down with WorldWorkerPool --
    // same double-destruction hazard already fixed once in ConnectionManager.
    _pool.reset();
    _initialized = false;
}

void WorldWorkerPool::submit(std::function<void()> task) {
    if (!_initialized) return;
    _pool->enqueue(std::move(task));
}