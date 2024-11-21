#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
 
class ThreadPool {
    public:
        ThreadPool(int num_threads);
        ~ThreadPool();
        void enqueue(std::function<void()> task);

    private:
        std::vector<std::thread> workerList;
        std::queue<std::function<void()>> taskQ;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stop;
        
        void workerThread();
}; 