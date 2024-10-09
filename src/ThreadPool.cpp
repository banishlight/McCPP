#include <ThreadPool>
#include <thread>

ThreadPool::ThreadPool(int num_threads) {
    for(int i = 0; i < num_threads; i++) {
        workers.emplace_back([this]() { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();
    for(std::thread& worker : workers) {
        worker.join()
    }
}

void enqueue(std::function<void()> task) {

}

void ThreadPool::workerThread() {

}

