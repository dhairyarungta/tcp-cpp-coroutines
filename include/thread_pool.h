#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <coroutine>
#include <condition_variable>
#include <functional>
#include <queue>

class ThreadPool
{
private:
    const int max_threads;
    bool is_shutdown;
    std::vector<std::thread> threads;
    std::queue<std::fucntion<void()>>jobs;
    std::mutex jobs_mutex;
    std::condition_variable jobs_condition;
    void run_jobs();

public:
    ThreadPool();
    ThreadPool(int max_threads);
    void shutdown();
    void run(std::function<void()>job);
    void run(std::coroutine_handle<>handle);

};