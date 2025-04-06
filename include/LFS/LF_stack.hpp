#pragma once
#include <algorithm>
#include <atomic>
#include "LF_allocator.hpp"


template<typename T>
class LF_stack : public FreeList<T>
{
    using FreeList<T>::head_;
    using FreeList<T>::counter_;
    using FreeList<T>::tail_;
    using Node = typename FreeList<T>::Node;

    protected:

    LF_allocator alloc_;

    public:

    LF_stack():
      alloc_([](void* data, size_t data_size)
      {
        std::destroy_at(static_cast<T*>(data));
      }, 2)
    {
      FreeList<T>::set_allocator(&alloc_);
    }

    ~LF_stack()
    {}


    protected:

    [[nodiscard]]
    std::optional<T> pop(bool isTop)
    {
      Node * old_head = head_.load(std::memory_order_relaxed);
      Node * tail = tail_.load(std::memory_order_relaxed); 
      auto  hzp =  alloc_.get_hazard();
      do
      {
        Node*  temp;
        do
        {
          temp = old_head;
          hzp->protect(old_head);
          old_head = head_.load(std::memory_order_relaxed);
        }
        while(old_head!=temp);
      }
      while(old_head!=tail && !head_.compare_exchange_strong(old_head, old_head->next_));
      
      std::optional<T> opt;
      if(old_head!=tail)
      {
        opt = std::move(old_head->data_);
        counter_.fetch_sub(1, std::memory_order_release);
      }
      
      if(!isTop)
      {
        alloc_.reclaim_hazard(std::move(hzp));
      }

      return opt;
    }

  public:

  [[nodiscard]]
  auto pop()
  {
    return pop(false);
  }

  [[nodiscard]]
  auto top()
  {
    return pop(true);
  }

};
