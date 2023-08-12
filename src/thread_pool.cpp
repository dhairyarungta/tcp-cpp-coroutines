#include "thread_pool.h"
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <coroutine>
#include <stdexcept>


ThreadPool::ThreadPool()
    :max_threads(std::thread::hardware_concurrency())
{
    is_shutdown = false;
}

ThreadPool::ThreadPool(int max_threads)
    :max_threads(max_threads)
{
    is_shutdown = false;
}

void ThreadPool::run_jobs()
{
    while(true)
    {
        std::function<void()>job;
        {

        }
    }
}

void ThreadPool::run(std::function<void()>job)
{
    {
        std::lock_guard<std::mutex> guard(job_mutex);
        if(is_shutdown)
        {
            throw std::runtime_error("ThreadPool::run() error: ThreadPool already shutdown");

        }
        jobs.push(job);

    }
}


void ThreadPool::run(std::coroutine_handle<>handle)
{

}

void ThreadPool::shutdown()
{
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        if(is_shutdown)
        {
            throw std::runtime_error("ThreadPool::shutdown error: ThreadPool already shut down");

        }
        is_shutdown = true;
    }
    jobs_condition.notify_all();
    for(auto it= threads.begin();it!=thread.end();++it)
    {
        it->join();
    }
}