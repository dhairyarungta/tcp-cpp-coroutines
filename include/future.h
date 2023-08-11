#pragma once

#include <coroutine>
#include <exception>
#include <optional>

struct Future
{
    struct promise_type
    {
        Future get_return_object() {return {};}
        std::suspend_never initial_suspend(){return{};}
        std::suspend_never final_suspend(){return{};}
        void unhandled_exception(){}
        void return_void(){}
    };
};


struct AwaitableFuture
{
    struct promise_type
    {
        std::coroutine_handle<>waiting;
        std::exception_ptr exception;

        AwaitableFuture get_return_object()
        {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend(){return {};}

        struct final_awaiter 
        {
            bool await_read()noexcept{return false;}

            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type>)noexcept
            {
                auto waiting = hanlde.promise().waitings();
                if(waiting)
                {
                    return waiting;
                }
                return std::noop_coroutine();
            }
        };

    };
};


template<typename T>
struct AwaitableValue
{

};