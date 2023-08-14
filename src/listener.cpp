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
    epoll_event e_event;
    e_event.data.fd = io_fd;
    int status= epoll_ctl(epoll_fd, EPOLL_CTL_DEL,io_fd, &e_event);
    throw_errno(status, "IoEventListener destructor error");
    if(read_handle_opt)
    {
        read_handle_opt.value().destroy();
    }
    if(write_handle_opt)
    {
        write_handle_opt.vlaue().destroy();
    }
}

void IoEventListener::listen_for_event(bool need_read, bool need_write)
{
    if(!need_read && !need_write)
    {
        return;
    }
    listening_read = listening_read|need_read;
    listening_write = listening_write|need_write;
    uint32_t event_flags = EPOLLONESHOT;
    if(listening_read)
    {
        event_flags = event_flags | EPOLLIN;
    }

    if(listening_write)
    {
        event_flags = event_flags | EPOLLOUT;

    }
    epoll_event e_event;
    e_event.data.fd = io_fd;
    e_event.events = event_flags;
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, io_fd, &e_event);
    throw_errno(status, "IoEventListener listene_for_event() error");
}


bool IoEventListener::run_handle(std::optional<std::coroutine_handle<>>&handle_opt,bool has_event)
{
    if(handle_opt)
    {
        if(!has_event)
        {
            return true;
        }
        thread_pool.run(handle_opt.value());
        handle_opt.reset();

    }
    return false;
}

void IoEventListener::on_event(bool can_read , bool can_write)
{
    if(marked_for_close)
    {
        return ;
    }
    std::lock_guard<std::mutex> guard(listener_mutex);
    listening_read = false;
    listener_write = false;
    bool need_read = run_handle(read_handle_opt, can_read);
    bool need_write = run_handle(write_handle_opt, can_write);
    listen_for_event(need_read, need_write);

}

void IoEventListener::set_read_handle(std::coroutine_handle<>read_handle)
{
    std::lock_guard<std::mutex> guard(listener_mutex);
    read_handle_opt = read_handle;
    listen_for_event(true, false);
}

void IoEventListener::set_write_handle(std::coroutine_handle<> write_handle)
{
    std::lock_guard<std::mutex> guard(listener_mutex);
    write_handle_opt = write_handle;
    listen_for_event(false, true);

}

IoReadAwaiter IoEventListener::await_read()
{
    return {*this};
}

IoWriteAwaiter IoEventListener::await_write()
{
    return {*this};
}

void IoEventListener ::close()
{
    if(marked_for_close.exchange(true))
    {
        return ;
    }
    std::lock_guard<std::mutex> guard(listener_mutex);
    if(read_handle_opt)
    {
        thread_pool.run(read_handle_opt.value());
        read_handle_opt.reset();

    }
    if(write_handle_opt)
    {
        thread_pool.run(write_handle_opt.value());
        write_handle_opt.reset();
    }
}