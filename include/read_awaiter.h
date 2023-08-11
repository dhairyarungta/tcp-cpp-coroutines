#pragma once
#include <coroutine>

class IoEventListener;

struct IoReadAwaiter
{
    IoEventListener& listener;
    bool await_read()noexcept;
    void await_suspend(std::coroutine_handle<>handle);
    void await_resume();
};