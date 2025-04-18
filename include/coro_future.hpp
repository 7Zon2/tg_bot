 #pragma once
 #include <iostream>
 #include <coroutine>
 #include <thread>
 #include <chrono>
 #include <future>
 #include <type_traits>
 #include <stdexcept>
 #include <stacktrace>

 template<typename T, typename...Args>
 requires(std::is_constructible_v<T>)
 struct std::coroutine_traits<std::future<T>, Args...>
 {  
    struct promise_type : std::promise<T>
    {
        std::future<T> get_return_object() noexcept
        {
            return this->get_future();
        }


        std::suspend_never 
        initial_suspend() const noexcept
        {
            return{};
        }


        std::suspend_never
        final_suspend() const noexcept   
        {
            return {};
        }


        void return_value(T& value) noexcept
                                    (std::is_nothrow_copy_constructible_v<T>)
        {
            this->set_value(value);
        }


        void return_value(T&& value) noexcept
                                     (std::is_nothrow_move_constructible_v<T>)
        {
            std::cout<<"return_value\n";
            this->set_value(std::move(value));
        }  


        void unhandled_exception() noexcept
        {      
            std::cerr<<std::stacktrace::current()<<std::endl;
            this->set_exception(std::current_exception());
        }
    };      
 };



template<typename...Args>
struct std::coroutine_traits<std::future<void>, Args...>
{
    struct promise_type : std::promise<void>
    {
        std::future<void> 
        get_return_object()
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {};}

        std::suspend_never final_suspend() const noexcept {return {};}


        void 
        return_void() noexcept
        {
            this->set_value();
        }


        void
        unhandled_exception() noexcept
        {   
            std::cerr<<std::stacktrace::current()<<std::endl;
            this->set_exception(std::current_exception());
        }
    };
};


template<typename T>
auto operator co_await (std::future<T> future) noexcept
{
    struct awaiter : std::future<T>
    {
        bool await_ready() const noexcept
        {
            using namespace std::chrono_literals;
            return this->wait_for(0s) != std::future_status::timeout;
        }


        void
        await_suspend(std::coroutine_handle<> coro) const
        {
                this->wait();
                coro();
        }

        T await_resume()
        {
            return this->get();
        }
    };

    return awaiter{std::move(future)};
}
