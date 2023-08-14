#include "thread_pool.h"
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <coroutine>
#include <stdexcept>


ThreadPool::ThreadPool()
    :max_threads(std::thread::hardware_concurrency()), is_shutdown(false)
{}

ThreadPool::ThreadPool(int max_threads)
    :max_threads(max_threads), is_shutdown(false)
{}

void ThreadPool::run_jobs()
{
    while(true)
    {
        std::function<void()>job;
        
        {
            std::unique_lock<std::mutex>jobs_lock(jobs_mutex);
            jobs_condition.wait(jobs_lock, [&]{return !jobs.empty()||is_shutdown;});
            if(jobs.empty()&& is_shutdown)
            {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        try
        {
            job();
        }
        catch(std::runtime_error err)
        {
            std::cerr << err.what() << std::endl;
        }
        
    }
}

void ThreadPool::run(std::function<void()>job)
{
    {
        std::lock_guard<std::mutex> guard(jobs_mutex);
        if(is_shutdown)
        {
            throw std::runtime_error("ThreadPool::run() error: ThreadPool already shutdown");
        }
        jobs.push(job);
        if(threads.size()<max_threads)
        {
            threads.push_back(std::thread(&ThreadPool::run_jobs,this));
            //thread = operator moves the thread (move semantics)
        }
    }
    jobs_condition.notify_one();
}


void ThreadPool::run(std::coroutine_handle<>handle)
{
    run([&,handle](){handle.resume();});
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
    for(auto it= threads.begin();it!=threads.end();++it)
    {
        it->join();
    }
}

