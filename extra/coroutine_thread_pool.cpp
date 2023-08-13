//this is a sample code for my understanding of how the threadpool 
//running coroutines probably works
//this code doesn't compile 

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <coroutine>

template <typename T>
class Future {
public:
    struct Promise {
        Promise() : value(nullptr) {}
        Future get_return_object() {
            return Future{std::coroutine_handle<Promise>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(const T& val) {
            value = new T(val);
        }
        void unhandled_exception() { std::terminate(); }

        T* value;
    };

    Future(std::coroutine_handle<Promise> handle) : handle(handle) {}
    ~Future() {
        if (handle) handle.destroy();
    }
    
    T get() {
        return *(handle.promise().value);
    }

private:
    std::coroutine_handle<Promise> handle;
};

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back([this] { WorkerThread(); });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    template <typename Func>
    Future<void> Enqueue(Func func) {
        using ReturnType = std::invoke_result_t<Func>;

        struct Task {
            Func func;
            std::coroutine_handle<> coroutine;
        };

        auto task = std::make_unique<Task>(Task{std::move(func), nullptr});
        auto future = task->coroutine.promise().get_return_object();

        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.push(std::move(task));
        }

        condition.notify_one();

        return future;
    }

private:
    void WorkerThread() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop) return;

            auto task = std::move(tasks.front());
            tasks.pop();

            lock.unlock();

            task->coroutine = std::coroutine_handle<>::from_address(nullptr);
            task->func();
            task->coroutine.resume();

            lock.lock();
        }
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::unique_ptr<Task>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    ThreadPool threadPool(4);

    auto task1 = threadPool.Enqueue([]() -> Future<int> {
        co_return 42;
    });

    auto task2 = threadPool.Enqueue([]() -> Future<int> {
        co_return 73;
    });

    std::cout << "Task 1 result: " << task1.get() << std::endl;
    std::cout << "Task 2 result: " << task2.get() << std::endl;

    return 0;
}
