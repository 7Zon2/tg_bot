#pragma once
#include <concepts>
#include <functional>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <string_view>

template<typename...Types>
void print(Types&&...args);

//#define PRINT_ON

#ifdef PRINT_ON
#define PRINT(...) print(__VA_ARGS__)
#else
#define PRINT(...)
#endif


template<typename T>
concept as_string_view = std::is_convertible_v<T, std::string_view>;

template<typename T>
concept as_wstring_view = std::is_convertible_v<T, std::wstring_view>;

template<typename T>
concept is_cout = requires (T && t)
{
    std::cout<<t;
};

template<typename T>
concept is_wcout = requires(T&& t)
{
    std::wcout<<t;
};

template<typename R>
concept is_range=std::ranges::viewable_range<R>;

template<typename T>
concept is_printable = requires(T&& t)
{
    requires 
    (
        is_cout<T>  ||
        is_wcout<T> ||
        as_string_view<T> ||
        as_wstring_view<T> ||
        is_range<T>
    );
};


template<typename  R>
concept is_string_container = 
std::ranges::viewable_range<R> 
&&
requires(R &&r)
{
    {*r.begin()} -> std::convertible_to<std::string_view>;
};


struct expander
{
    template<typename...Types>
    expander([[maybe_unused]] Types&&...args){}
};


template<typename...Types>
struct adapter : Types...
{
    using Types::operator()...;   
};



template<typename...Types>
adapter(Types...) -> adapter<Types...>;



template<typename...Types>
inline void print(Types&&...args)
{

     adapter mycaller
    {
        []<is_printable T>
        (T&& t)
        {
            if constexpr(is_cout<T>)
                std::cout<<t;
            else if constexpr(is_wcout<T>)
                std::wcout<<t;
            else if constexpr (as_string_view<T>)
                std::cout<<std::string_view{t};
            else if constexpr (as_wstring_view<T>)
                std::wcout<<std::wstring_view{t};
            else if constexpr(is_range<T>)
                std::ranges::for_each(std::ranges::ref_view(t), [](auto&& obj){std::cout<<obj<<"\n";});
        },

        [](std::string_view vw,const size_t sz)
        {
            std::cout<<vw.substr(0,sz);
        },

        [](std::wstring_view vw,const size_t sz)
        {
            std::wcout<<vw.substr(0,sz);
        }
    };


    auto call_print=[&mycaller]<typename Arg>
    (Arg&& arg)
    {
        std::invoke(mycaller,std::forward<Arg>(arg));
    };
 
    expander{(call_print(std::forward<Types>(args)),void(),0)...};
}



template<typename...Types>
requires requires(Types&&...args)
{
   print(std::forward<Types>(args)...);
}
std::string all_to_string(Types&&...args)
{
    auto parse=[]<typename T>
    (std::string& str, T&& t)
    {
        std::string_view view=t;
        str.append(view);
    };

    std::string str;
    (parse(str,std::forward<Types>(args)),...);
    return str;
}


