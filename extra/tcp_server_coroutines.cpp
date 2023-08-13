#include <iostream>
#include <vector>
#include <thread>
#include <coroutine>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
//overview of a tcp server using epoll , coroutines and threadpool


struct Coroutine
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    struct promise_type 
    {
        Coroutine get_return_objet()
        {
            return Coroutine(handle_type::from_promise(*this));

        }
        std::suspend_never initial_suspend(){return{};}
        std::suspend_never final_suspend(){return{};}
        void return_void(){}
        void unhandled_exception(){}
    };

    handle_type h_;

    Coroutine(handle_type h):h_(h){}
    ~Coroutine()
    {
        if(h_)
            h_.destroy();
    }
    void resume()
    {
        if(h_)
            h_.resume();
    }
};

class ThreadPool
{
public :
    ThreadPool(int numThreads) : stop_(false) {
        for (int i = 0; i < numThreads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &thread : threads_) {
            thread.join();
        }
    }

    template <typename F>
    void enqueue(F &&task) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace(std::forward<F>(task));
        }
        condition_.notify_one();
    }

private :
    std::vector<std::thread>threads_;
    std::queue<std::function<void()>>tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_;
};


class TCPServer
{
public:
    TCPServer(int port , int numThreads):port_(port), pool_(numThreads){}
    void Start()
    {
        // TODO: Set up listening socket, epoll, and event loop
        // Create coroutine to accept new clients and handle them
        Coroutine coroutine = AcceptClients();
        pool_.enqueue([coroutine]()mutable
        {
            coroutine.resume();
        })
    }
private :
    int port_;
    ThreadPool pool_;

    Coroutine AcceptClients()
    {
        whle(true)
        {
            // Wait for incoming connections using epoll
            int clientSocket = -1;
            if (clientSocket!=-1)
            {
                Coroutine coroutine = HandleClient(pool_, clientSocket);
                pool_.enqueue([coroutine]()mutable{
                    coroutine.resume();
                });
                

            }

        }
    }
};


int main()
{    
    TCPServer server(8080, 4);
    server.Start();
    return 0;


}