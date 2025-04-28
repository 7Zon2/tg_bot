#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include "LFS/LF_FreeList.hpp"
#include "LFS/share_resource.hpp"
#include "LF_allocator.hpp"


template<typename T>
class LF_stack : public FreeList<T>
{
    protected:

    using FreeList_t = FreeList<T>;
    using FreeList_t::head_;
    using FreeList_t::counter_;
    using FreeList_t::tail_;
    using Node = typename FreeList_t::Node;
  
    protected:

    std::shared_ptr<LF_allocator> alloc_;

    public:

    LF_stack():
      alloc_
      (
       std::make_shared<LF_allocator>
       (  
          [](void* data, size_t sz)
          {
            Node* node = static_cast<Node*>(data);
            std::destroy_at(&node->data_);
            ShareResource::res_.deallocate(data,sz);
          },
          2
       )
      )
      {
        FreeList_t::set_allocator(alloc_);
      }


    ~LF_stack()
    {}

    public:

    void pop()
    {
      auto * node = FreeList_t::pop();
      if(node)
      {
        alloc_->deallocate(node, sizeof(Node));
      }
    }


  [[nodiscard]]
  std::optional<T> top()
  {
    Node * temp{};
    Node * old_head = head_.load(std::memory_order_relaxed);
    auto hzp = alloc_->get_hazard();
    do
    {
      temp = old_head;
      hzp->protect(old_head);
      old_head = head_.load(std::memory_order_relaxed);
    }
    while(old_head!=temp);

    std::optional<T> opt;
    if(old_head != tail_.load(std::memory_order_relaxed))
    {
      opt = old_head->data_; 
    }

    return opt;
  }

};
