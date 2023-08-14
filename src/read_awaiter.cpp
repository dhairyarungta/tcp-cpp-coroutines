#include "../include/read_awaiter.h"
#include "../include/listener.h"
#include <coroutine>

bool IoReadAwaiter::await_read()noexcept
{
    return false;

}

void IoReadAwaiter::await_suspend(std::coroutine_handle<>handle)
{
    listener.set_read_handle(handle);
}

void IoReadAwaiter::await_resume()
{

}

