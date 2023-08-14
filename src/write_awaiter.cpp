#include "../include/write_awaiter.h"
#include "../include/listener.h"
#include <coroutine>

bool IoWriteAwaiter::await_ready()noexcept
{
    return false;
}

void IoWriteAwaiter::await_suspend(std::coroutine_handle<> handle)
{
    listener.set_write_handle(handle);
}

void IoWriteAwaiter::await_resume()
{
    
}