#include <thread>
#include <ThreadPool.hpp>
#include <Console.hpp>

ThreadPool::ThreadPool(int num_threads) : stop(false) {
    for(int i = 0; i < num_threads; i++) {
        workerList.emplace_back([this]() { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    Console::getConsole().Entry("Closing ThreadPool");
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();
    for(std::thread& worker : workerList) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskQ.push(std::move(task));
    }
    cv.notify_one();
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]() {return stop || !taskQ.empty(); });
            if (stop && taskQ.empty()) {
                return;
            }
            task = std::move(taskQ.front());
            taskQ.pop();
        }
        task();
    }
}

