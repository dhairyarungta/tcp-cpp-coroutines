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
        using ReturnType = std::invoke_result_t<Func>;
        struct Task 
        {
            Func func;
            std::coroutine_handle<> coroutine;
        };

        std::coroutine_handle<> coroutine= std::coroutine_handle<>::from_address(nullptr);
        auto task = std::make_unique<Task> (Task{std::move(func), coroutine});
        auto future = task->coroutine.promise().get_return_object();
        {
            std::unique_lock<std::mutex>lock(mutex_);
            tasks.push(std::move(task));

        }
        condition_.notify_all();
        return future;

    }
private :
    void WorkerThread()
    {
        while(true)
        {
            std::unique_ptr<Task> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]{return stop||!tasks.empty();});
                if(stop&& tasks.empty())
                {
                    return;
                }
                task = std::move(task.front());
                tasks.pop();

            }
            task->func();
            task->coroutine.resume();
        }
    }

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

    auto asyncTask = [](int id)->std::coroutine_handle<>
    {
        std::cout<<"Async Task "<<id<<started<<std::endl;
        co_await std::suspend_always();
        std::cout<<<"Async task "<<id<< "completed"<<std::endl;
    };
    std::vector<std::coroutine_handle<>> coroutines;
    for (int i =0;i<10;++i)
    {
        coroutines.push_back(threadPool.Enqueue([i,asyncTask]()mutable{
            asyncTask(i);
        }));
    }

    for (auto coroutine : coroutines)
    {
        coroutine.resume();
    }
    return 0;
    
}