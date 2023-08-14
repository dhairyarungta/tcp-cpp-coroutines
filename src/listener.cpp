#include "../include/listener.h"
#include "../include/thread_pool.h"
#include "../include/error.h"
#include <cstdint>
#include <sys/epoll.h>


IoEventListener::IoEventListener(ThreadPool& threadPool, int epoll_fd, int io_fd)
    :thread_pool(thread_pool), epoll_fd(epoll_fd), io_fd(io_fd), marked_for_close(false),
    listening_read(false), listening_write(false)
{
    epoll_event  e_event;
    e_event.data.fd = io_data;
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, io_fd, &e_event);
    throw_errno(status,"IoEventListener constructor error: ");
}

IoEventListener::~IoEventListener()
{

}


IoEventListener::await_read()
{
    return {*this};
}

IoEventListener::await_write()
{
    return {*this};
}

