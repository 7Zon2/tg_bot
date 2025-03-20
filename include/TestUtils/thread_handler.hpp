#pragma once
#include "print.hpp"
#include <chrono>


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

}//ThreadHandler
