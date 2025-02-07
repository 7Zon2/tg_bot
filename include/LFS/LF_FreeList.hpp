#pragma once
#include <memory_resource>
#include <atomic>
#include "share_resource.hpp"

template<typename T>
class FreeList
{
    struct Node
    {
        T data_{};
        Node* next_{};

        public:

        template<typename U>
        Node(U&& data):
            data_(std::forward<U>(data)){}

        T& get() noexcept 
        {
            return data_;
        }
    };

    protected:

    std::pmr::memory_resource* res_{};
    std::atomic<Node*> tail_{};
    std::atomic<Node*> head_{};
    std::atomic<size_t> counter_{};

    public:

    using node_type = Node;

    FreeList(std::pmr::memory_resource* res = &ShareResource::res_):
      res_(res)
    {
      set_allocator(res_);
    }


    virtual 
    ~FreeList()
    {
      clear();
    }


    FreeList(FreeList&& rhs) noexcept
    {
      rhs.tail_  = tail_.exchange(rhs.tail_, std::memory_order_relaxed);
      rhs.head_  = head_.exchange(rhs.head_, std::memory_order_relaxed);
      rhs.counter_ = counter_.exchange(rhs.counter_, std::memory_order_relaxed);
    }


    FreeList& operator = (FreeList&& rhs) noexcept
    {
      if (&rhs != this)
      {
        rhs.tail_  = tail_.exchange(rhs.tail_, std::memory_order_relaxed);
        rhs.head_ =  head_.exchange(rhs.head_, std::memory_order_relaxed);
        rhs.counter_ = counter_.exchange(rhs.counter_, std::memory_order_relaxed);
      }
      return *this;
    }

    public:

    void set_allocator(std::pmr::memory_resource* res = &ShareResource::res_)
    {
      clear();
      res_ = res;
      void* ptr = res_->allocate(sizeof(node_type));
      tail_.store(::new(ptr) node_type(T{}), std::memory_order_relaxed);
      head_.store(tail_, std::memory_order_relaxed);
    }


    auto* get_allocator() const noexcept 
    {
      return res_;
    }


    void clear()
    {
      auto * tail = tail_.exchange(nullptr, std::memory_order_relaxed);
      auto * head = head_.exchange(nullptr , std::memory_order_relaxed);
      while(head!=tail)
      {
        auto * next = head->next_;
        res_->deallocate(head, sizeof(node_type));
        head = next;
      }
    }


    Node* pop() noexcept
    { 
      Node* ptr = head_.load(std::memory_order_relaxed);
      Node* tail = tail_.load(std::memory_order_relaxed);
      while(ptr!=tail && !head_.compare_exchange_strong(ptr, ptr->next_, std::memory_order_relaxed))
      {
        counter_.fetch_sub(1, std::memory_order_release);
      }
      return (ptr != tail) ? ptr : nullptr;
    }


    void push(Node* node) noexcept
    {
      node->next_ = head_.load(std::memory_order_relaxed);
      while(!head_.compare_exchange_weak(node->next_, node, std::memory_order_relaxed));
      counter_.fetch_add(1, std::memory_order_release);
    }


    template<typename U>
    Node* push(U&& data)
    {
      void * ptr = res_->allocate(sizeof(Node));
      Node* node = new (ptr) Node{std::forward<U>(data)};
      push(node);
      return node;
    }


    void merge
    (FreeList&& list) noexcept
    {  
      if(&list == this)
      {
        return;
      }

      FreeList rhs{std::move(list)};
      Node* old_tail = tail_.load(std::memory_order_relaxed);
      old_tail->next_ = rhs.head_.load(std::memory_order_relaxed);
      tail_.store(rhs.tail_, std::memory_order_relaxed);
    }


    size_t size() const noexcept
    {
      if(!head_.load(std::memory_order_relaxed))
      {
        return 0;
      }
      return counter_.load(std::memory_order_acquire);
    }

};

template<typename T>
FreeList(T) -> FreeList<T>;


