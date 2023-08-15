#include "../include/monitor.h"
#include "../include/listener.h"
#include "../include/thread_pool.h"
#include "../include/error.h"
#include <array>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

IoEventMonitor::IoEventMonitor(ThreadPool& thread_pool)
    :thread_pool_(thread_pool)
{
    is_shutdown = false;
    epoll_fd = epoll_create(1);
    thread_pool.run([&]()
    {
        this->run();                
    });
}
void IoEventMonitor::run()
{
    std::array<epoll_event,IO_POLL_MAX_EVENTS>events;
    while(!is_shutdown)
    {
        int count = epoll_wait(epoll_fd,events.data(), IO_POLL_MAX_EVENTS, IO_POLL_TIMEOUT);
        if(count<0)
        {
            if(errno==EINTR)
            {
                continue;
            }
            throw_errno(count , "IoEventMontior run() error");
        }
        trigger_events(events.data(), count);
    }
}
void IoEventMonitor::shutdown()
{
    if(is_shutdown.exchange(true))
    {
        throw std::runtime_error("IoEventMonitor shutdown() error: already shut down");

    }
    fd_listener_map.clear();
    close(epoll_fd);
}
IoEventListener* IoEventMonitor::register_fd(int fd)
{
    auto listener = std::make_unique<IoEventListener>(thread_pool,epoll_fd, fd);
    auto listener_p = listener.get();
    {
        std::lock_guard<std::mutex> guard(fd_mutex);
        fd_listener_map[fd]= std::move(listener);
    }
    return listener_p;
}

void IoEventMonitor::remove_fd(int fd)
{
    std::lock_guard<std::mutex> guard(fd_mutex);
    fd_listener_map.erase(fd);
    //TODO : remove fd of from the epoll fd
}   
void IoEventMonitor::trigger_events(epoll_event*events, int count)
{
    std::lock_guard<std::mutex> guard(fd_mutex);
    for (int i = 0;i<count;++i)
    {
        epoll_event* event = *(events + i);
        auto it = fd_listener_map.find(event.data.fd);
        if(it==fd_listener_map.end()) 
        {
            continue;
        }
        bool can_read = (event.events & EPOLLIN)==EPOLLIN;
        bool can_write = (event.events & EPOLLOUT)==EPOLLOUT;
        it->second->on_event(can_read, can_write);
    }
}

