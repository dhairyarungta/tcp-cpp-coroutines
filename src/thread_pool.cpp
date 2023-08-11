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

}


void ThreadPool::run(std::coroutine_handle<>handle)
{

}

void ThreadPool::shutdown()
{

}