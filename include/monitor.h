#pragma once

#include "listener.h"
#include <atomic>
#include <memory>
#include <unordered_map>
#include <sys/epoll.h>
#include <mutex>


#define IO_POLL_MAX_EVENTS 1000
#define IO_POLL_TIMEOUT 2000


class IoEventListener;
class ThreadPool;

class IoEventMonitor
{
private :
    ThreadPool  & threadpool;
    std::atomic_bool is_shutdown;
    std::mutex fd_mutex;
    std::unordered_map<int, std::Unique_ptr<IoEventListener>>fd_listener_map;
    int epoll_fd;
    void run();
    void trigger_events(epoll_event* events, int count);

public:
    IoEventMonitor(ThreadPool& thread_poll);
    void shutdown();
    IoEventListener* register_fd(int fd);
    void remove_fd(int fd);
};