#pragma once
#include <algorithm>
#include "LF_allocator.hpp"


template<typename T>
class LF_stack : public FreeList<T>
{
    using FreeList<T>::head_;
    using FreeList<T>::counter_;
    using FreeList<T>::tail_;
    using Node = typename FreeList<T>::node_type;

    protected:

    LF_allocator alloc_;

    public:

    LF_stack():
      alloc_([](void* data, size_t data_size)
      {
        std::destroy_at(static_cast<T*>(data));
      })
    {
      FreeList<T>::set_allocator(&alloc_);
    }

    ~LF_stack()
    {}


    public:

    [[nodiscard]]
    std::optional<T> pop()
    {
      Node * old_head = head_.load(std::memory_order_relaxed);
      Node * tail = tail_.load(std::memory_order_relaxed); 
      auto * hzp = alloc_.get_hazard();
      do
      {
        Node*  temp;
        do
        {
          temp = old_head;
          old_head = head_.load(std::memory_order_relaxed);
          hzp->protect(old_head, sizeof(Node));
        }
        while(old_head!=temp);
      }
      while(old_head!=tail && !head_.compare_exchange_strong(old_head, old_head->next_));
      
      std::optional<T> opt;
      if(old_head!=tail)
      {
        opt = std::move(old_head->get());
      }
      alloc_.deallocate_hazard(hzp);
      return opt;
    }
};
