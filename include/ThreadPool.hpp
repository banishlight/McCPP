#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>

class ThreadPool {
    public:
        ThreadPool();
        ~ThreadPool();
        void enqueue(std::function<void()> task);
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable cv;

        void workerThread();
};