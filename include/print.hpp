#pragma once
#include <concepts>
#include <ranges>
#include <string_view>


template<typename R>
concept is_range=std::ranges::viewable_range<R>;

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
adapter(Types...)->adapter<Types...>;



template<typename...Types>
inline void print(Types&&...args)
{

     adapter mycaller
    {

        [](std::string_view vw="\n")
        {
            std::cout<<vw;
        },

        [](std::wstring_view vw=L"\n")
        {
            std::wcout<<vw;
        },

        [](std::string_view vw,const size_t sz)
        {
            std::cout<<vw.substr(0,sz);
        },

        [](std::wstring vw,const size_t sz)
        {
            std::wcout<<vw.substr(0,sz);
        },

        []<typename T>
        requires requires (T&& t)
        {
            std::to_wstring(t);
        }
        (T&& t)
        {
            std::wcout<<std::to_wstring(t);
        },

        []<is_string_container R> 
        (R&& r)
        {
            for(auto it=r.begin();it!=r.end();it++)
            {
                std::string_view vw{*it};
                std::cout<<vw;
            }
        },

        []<std::ranges::viewable_range R>
        requires std::is_convertible_v<R,std::wstring>
        (R&& r)
        {
            std::ranges::ref_view Ran=r;
            std::wcout<<std::wstring{Ran.begin(),Ran.end()};
        },

        []<typename It>
        requires requires (It it)
        {
            requires  std::is_base_of_v<std::forward_iterator_tag,It>;
            requires  std::is_convertible_v<decltype(*it),wchar_t>;
        }
        (It beg,It end)
        {
            std::wcout<<std::wstring{beg,end};
        },

        []<typename T>
        requires requires(T&& t)
        {
            t.begin();
            t.end();

            std::cout<<*t.begin();
        }
        (T&& t)
        {
            std::string_view str{t.begin(),t.end()};
            std::cout<<str;
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
    (std::string& str,T&& t)
    {
        std::string_view view=t;
        str.append(view);
    };

    std::string str;
    (parse(str,std::forward<Types>(args)),...);
    return str;
}
