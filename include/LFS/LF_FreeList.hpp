#pragma once
#include <cstddef>
#include <iterator>
#include <memory_resource>
#include <atomic>
#include <cassert>
#include <type_traits>
#include "share_resource.hpp"

template<typename T, bool AtomicNode = false>
struct Node_Type
{
  using Node = Node_Type;
  T data_{};
  Node* next_{};

  public:

  Node_Type(){}

  template<typename U>
  Node_Type(U&& data):
        data_(std::forward<U>(data)){}

  auto next() noexcept
  {
    return next_;
  }
};


template<typename T>
struct Node_Type<T,true>
{
  using Node = Node_Type;
  T data_{};
  std::atomic<Node*> next_{};

  public:

  Node_Type(){}

  template<typename U>
  Node_Type(U&& data):
        data_(std::forward<U>(data)){}

  auto next() noexcept
  {
    return next_.load(std::memory_order_relaxed);
  }
};



template<typename T, bool AtomicNode = false>
class FreeList
{
    public:

    using Node = Node_Type<T, AtomicNode>;

    protected:

    class FL_iterator 
    {
      protected:

      Node* node_;

      public:
      
      FL_iterator(){}
      FL_iterator(Node* node):
        node_(node){}

      FL_iterator(const FL_iterator&) = default;
      FL_iterator(FL_iterator&&) = default;

      FL_iterator& operator = (const FL_iterator&) = default;
      FL_iterator& operator = (FL_iterator&&) = default;
     
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::remove_cvref_t<T>;
      using pointer = T*;
      using reference = T&;
      using difference_type = std::ptrdiff_t;

      public:

      friend bool operator ==(const FL_iterator& it, const Node* node) noexcept
      {
        return it.node_ == node;
      }

      friend bool operator ==(const FL_iterator&, const FL_iterator&) noexcept = default;


      FL_iterator& 
      operator ++() noexcept
      {
        if(node_)
        {
          node_ = node_->next();
        }
        return *this;
      }

      FL_iterator
      operator ++(int) noexcept
      {
        FL_iterator it(node_);
        if(node_)
        {
          node_ = node_->next();
        }
        return it;
      }

      reference
      operator *() const noexcept
      {
        return node_->data_;
      }

      pointer
      operator ->() const noexcept
      {
        return &node_->data_;
      }
    };

    static_assert(std::forward_iterator<FL_iterator>);

    protected:

    std::pmr::memory_resource* res_{};
    std::atomic<Node*> head_{};
    std::atomic<Node*> tail_{};
    std::atomic<size_t> counter_{};
    
    protected:

    void purge()
    {
      void * p = allocate();
      Node * new_head = ::new(p) Node();
      auto * head = head_.exchange(new_head, std::memory_order_relaxed);
      tail_.exchange(head_, std::memory_order_relaxed);
      while(head)
      {
        auto * next = head->next();
        res_->deallocate(head, sizeof(Node));
        head = next;
      }
      counter_.store(0, std::memory_order_release);
    }

    void* allocate()
    {
      return res_->allocate(sizeof(Node));
    }

    void initialize()
    {
      void* ptr = allocate();
      tail_.store(::new(ptr) Node(), std::memory_order_relaxed);
      head_.store(tail_, std::memory_order_release);
    }

    FreeList(Node* head, Node* tail, std::pmr::memory_resource* res):
      head_(head), 
      tail_(tail), 
      res_(res)
    {
      size_t counter =0;
      for(;head!=tail;head = head->next(), ++counter)
      {}
      counter_.store(counter, std::memory_order_relaxed);
    }

    public:

    using iterator = FL_iterator;

    FreeList(std::pmr::memory_resource* res = &ShareResource::res_):
      res_(res)
    {
      initialize();
    }


    virtual 
    ~FreeList()
    {
      purge();
      auto * head = head_.exchange(nullptr, std::memory_order_relaxed);
      [[maybe_unused]]
      auto * tail = tail_.exchange(nullptr, std::memory_order_relaxed);
      assert(head == tail);
      if(head)
      {
        res_->deallocate(head, sizeof(Node));
      }
    }


    FreeList(FreeList&& rhs) noexcept
    {
      assert(rhs.res_);
      res_ = rhs.res_;
      initialize();
      rhs.head_  = head_.exchange(rhs.head_, std::memory_order_acquire);
      rhs.tail_  = tail_.exchange(rhs.tail_, std::memory_order_relaxed);
      rhs.counter_ = counter_.exchange(rhs.counter_, std::memory_order_relaxed);
    }


    FreeList& operator = (FreeList&& rhs) noexcept
    {
      if (&rhs != this)
      {
        assert(rhs.res_);
        res_ = rhs.res_;
        initialize();
        rhs.head_  =  head_.exchange(rhs.head_, std::memory_order_acquire);
        rhs.tail_  = tail_.exchange(rhs.tail_, std::memory_order_relaxed);
        rhs.counter_ = counter_.exchange(rhs.counter_, std::memory_order_relaxed);
      }
      return *this;
    }

    public:

    [[nodiscard]]
    auto begin() noexcept
    {
      return iterator(head_.load(std::memory_order_relaxed));  
    }

    
    [[nodiscard]]
    auto end() noexcept
    {
      return iterator(tail_.load(std::memory_order_relaxed));
    }


    void set_allocator(std::pmr::memory_resource* res = &ShareResource::res_)
    {
      res_ = res;
    }


    [[nodiscard]]
    auto* get_allocator() const noexcept 
    {
      return res_;
    }


    virtual 
    void clear()
    {
      purge();
    }


    Node* pop() noexcept
    { 
      Node * head = head_.load(std::memory_order_relaxed);
      Node * tail = tail_.load(std::memory_order_relaxed);
      while(head!=tail && !head_.compare_exchange_strong(head, head->next_, std::memory_order_relaxed))
      {
        counter_.fetch_sub(1, std::memory_order_release);
      }

      Node* new_head = head_.load(std::memory_order_relaxed);
      return !tail_.compare_exchange_strong(new_head, new_head, std::memory_order_relaxed) ? head : nullptr;  
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


    void reset() noexcept
    {
      head_.store(nullptr, std::memory_order_relaxed);
      tail_.store(nullptr, std::memory_order_relaxed);
    }


    void merge
    (FreeList&& list) noexcept
    {  
      if(&list == this)
      {
        return;
      }

      FreeList rhs{std::move(list)};
      Node * old_tail;
      do
      {
        old_tail = tail_.load(std::memory_order_relaxed);
        old_tail->next_ = rhs.head_.load(std::memory_order_relaxed);
      }
      while(!tail_.compare_exchange_strong(old_tail, rhs.tail_, std::memory_order_relaxed));
      rhs.reset();
      counter_.fetch_add(rhs.size(), std::memory_order_release);
    }

    
    [[nodiscard]]
    FreeList 
    cut(Node* it) noexcept 
    {
      if(!it || !it->next())
      {
        return {};
      }

      Node* head = it->next_.exchange(nullptr, std::memory_order_relaxed);
      Node* tail = tail_.load(std::memory_order_acquire);
      FreeList temp{head, tail, res_};
      tail_.store(it, std::memory_order_release);
      counter_.fetch_sub(temp.size(), std::memory_order_release);
      return std::move(temp);
    }


    size_t size() const noexcept
    {
      Node* head = head_.load(std::memory_order_relaxed);
      Node* tail = tail_.load(std::memory_order_relaxed);
      if(head == tail)
      {
        return 0;
      }
      return counter_.load(std::memory_order_acquire);
    }
};

template<typename T>
FreeList(T) -> FreeList<T>;
