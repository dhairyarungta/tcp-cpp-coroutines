#pragma once

#include <coroutine>

class IoEventListener;

struct IoWriteAwaiter
{
    IoEventListener& listener;
    bool await_ready()noexcept;
    void await_suspend(std::coroutine_handle<>handle);
    void await_resume(std::coroutine_handle<>handle);
}