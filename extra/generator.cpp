//generator using c++20 coroutine

#include <iostream>
#include <coroutine>


template <typename T>
class RangeGenerator 
{

public: 
    struct promise_type
    {
        T current_value;    
        
        RangeGenerator<T> get_return_object()
        {
            return RangeGenerator{this};
        }
        
        std::suspend_never initial_suspend()
        {
            return{};
        }
        
        std::suspend_never final_suspend()noexcept
        {
            return {};
        }

        void return_void(){};

        std::suspend_always yield_value(T value)
        {
            current_value= value;
            return{};
        }        

        void unhandled_exception()
        {
            std::terminate();
        }


    };


    RangeGenerator(promise_type *promise)
        :promise_(promise),end_(promise->current_value){}

    bool next()
    {
        return !done() && (promise_->current_value<end_);
    }

    bool done()
    {
        return promise_->current_value>=end_;

    }

    T value()
    {
        return promise_->current_value;
    }

    void advance()
    {
        if(!done())
        {
            ++promise_->current_value;
        }
    }

private : 
    promise_type* promise_;
    T end_;
        

};


//coroutine fucntion
RangeGenerator<int> range(int begin, int end)
{
    for(int i =begin;i<end;++i)
    {
        co_yield i;
    }
}


int main()
{
    for (int value:range(1,10))
    {
        std::cout<<value<<" ";
    }
    std::cout<<std::endl;
    return 0;
}