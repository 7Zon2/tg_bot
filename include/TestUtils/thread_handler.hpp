#pragma once
#include "print.hpp"
#include <functional>
#include <chrono>
#include <cassert>

namespace ThreadHandler
{

using namespace std::chrono_literals;

class Thread_Timer
{
  using time_t = decltype(std::chrono::steady_clock::now());

  time_t start_;
  time_t end_;

  bool stopped_=false;

  public:

  Thread_Timer()
  {
    start_ = std::chrono::steady_clock::now();
  }

  void print_difference()
  {
    auto dif = std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_);
    print("\n\t", dif.count(),"ms\t\n");
  }

  void operator()()
  {
    stopped_ = true;
    end_ = std::chrono::steady_clock::now();
    print_difference();
  }
  
  void refresh()
  {
    stopped_ = false;
    start_ = std::chrono::steady_clock::now();
  }

  ~Thread_Timer()
  {
    if(!stopped_)
      operator()();
  }
};



class FuncHandler
{
  int limit_;

  using func_type = std::function<void(void)>;

  public:

  enum class LOOP_TYPE
  {
    SIMPLE = 0,
    INCREMENT = 1
  };

  FuncHandler(){}

  FuncHandler(int limit):
    limit_(limit)
  {
    assert(limit_>0);
  }


  template<LOOP_TYPE type, typename F, typename...Types>
  void go(F&& fun, Types&&...args)
  { 
    size_t limit = limit_;

    auto call = []<typename Fun, typename...TT>
    (Fun f, TT&&...args)
    {
      std::invoke(std::forward<Fun>(f), std::forward<TT>(args)...);
    };


    if constexpr (type == LOOP_TYPE::SIMPLE)
    {
      for(size_t i = 0; i < limit; i++)
        call(std::forward<F>(fun), std::forward<Types>(args)...);
    }

    if constexpr(type == LOOP_TYPE::INCREMENT)
    {
      for(size_t i = 0; i < limit; i++)
        call(std::forward<F>(fun),i, std::forward<Types>(args)...);
    }
  }

};

} //namespace ThreadHandler

