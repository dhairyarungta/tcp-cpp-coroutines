#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <coroutine>

class ThreadPool
{
public :
    ThreadPool(size_t numThreads): stop(false)
    {
        workers.reserver(numThreads);
        for(size_t i =0 ;i<numThreads;++i)
        {
            workers.emplace_back([this]{WorkerThread();});
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex>lock(mutex_);
            stop = true;
        }
        condition_.notify_all();
        for (std::thread& worker:workers)
        {
            worker.join();
        }
    }

    template<typename Func>
    auto Enqueue(Func func)
    {
        using 
    }
private :
    using Task = struct {std::function<void()>func, std::coroutine_handle<> coroutine;};
    std::vector<std::thread>workers;
    std::queue<std::unique_ptr<Task>>tasks;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop;
};

int main()
{
    ThreadPool threadPool(4);
    
}