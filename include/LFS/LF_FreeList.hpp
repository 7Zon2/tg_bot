#pragma once
#include "print.hpp"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory_resource>
#include <atomic>
#include <cassert>
#include <type_traits>
#include <utility>
#include "share_resource.hpp"

template<typename T, bool AtomicNode = false>
struct Node_Type
{
  using Node = Node_Type;
  T data_{};
  Node* next_{};

  public:

  template<typename...Types>
  Node_Type(Types&&...args) noexcept (std::is_nothrow_constructible_v<T>):
    data_(std::forward<Types>(args)...){}

  template<typename U>
  Node_Type(U&& data) noexcept (std::is_nothrow_constructible_v<T>):
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

  template<typename...Types>
  Node_Type(Types&&...args) noexcept (std::is_nothrow_constructible_v<T>):
    data_(std::forward<Types>(args)...){}

  template<typename U>
  Node_Type(U&& data) noexcept (std::is_nothrow_constructible_v<T>):
        data_(std::forward<U>(data)){}

  auto next(std::memory_order m = std::memory_order_relaxed) noexcept
  {
    return next_.load(m);
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
      
      FL_iterator() noexcept {}

      FL_iterator(Node* node) noexcept:
        node_(node){}

      FL_iterator(const FL_iterator&) noexcept = default;
      FL_iterator(FL_iterator&&) noexcept = default;

      FL_iterator& operator = (const FL_iterator&) noexcept = default;
      FL_iterator& operator = (FL_iterator&&) noexcept = default;
     
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::remove_cvref_t<T>;
      using pointer = T*;
      using reference = T&;
      using difference_type = std::ptrdiff_t;

      public:


      friend bool operator ==(const FL_iterator&, const FL_iterator&) noexcept = default;


      operator bool() noexcept
      {
        return node_;
      }


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
    std::atomic<int64_t> counter_{};
    
    protected:

    void purge()
    {
      auto * head = head_.load(std::memory_order_relaxed);
      while(head)
      {
        auto * next = head->next();
        res_->deallocate(head, sizeof(Node));
        head = next;
      }
      initialize();
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
      size_t counter = 0;
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
      auto * head = head_.exchange(nullptr, std::memory_order_relaxed);
      while(head)
      {
        Node* next = head->next();
        res_->deallocate(head, sizeof(Node));
        head = next;
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


    [[nodiscard]]
    auto data(std::memory_order  mod = std::memory_order_acquire) const noexcept 
    {
      return head_.load(mod);
    }


    [[nodiscard]]
    auto back(std::memory_order mod = std::memory_order_acquire) const noexcept 
    {
      return tail_.load(mod);
    }


    [[nodiscard]]
    std::atomic<Node*>& 
    head() noexcept
    {
      return head_;
    }


    [[nodiscard]]
    std::atomic<Node*>& 
    tail() noexcept
    {
      return tail_;
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
      Node* next = head->next();

      while
      (
        (next && !head_.compare_exchange_strong(head, next, std::memory_order_release))
      )
      {
        next = head->next();
      }

      assert(head && tail);

      if(!next)
      {
        return nullptr;
      }

      counter_.fetch_sub(1, std::memory_order_release);
      return head;  
    }


    template<typename...Types>
    [[nodiscard]]
    Node* create_node(Types&&...args) 
    {
      void * ptr = res_->allocate(sizeof(Node));
      Node* node = new(ptr) Node{std::forward<Types>(args)...};
      return node;
    }


    void push(Node* node) noexcept
    {
      node->next_ = head_.load(std::memory_order_relaxed);
      Node * next = node->next();
      while(!head_.compare_exchange_strong(next, node, std::memory_order_relaxed))
      {
        node->next_ = head_.load(std::memory_order_relaxed);
        next = node->next();
      }
      counter_.fetch_add(1, std::memory_order_release);
    }

   
    template<typename...Types>
    Node* push(Types&&...args)
    {

      Node* node = create_node(std::forward<Types>(args)...);
      push(node);
      return node;
    }


    void push_back(Node* node) noexcept
    {
      Node* old_tail = tail_.load(std::memory_order_relaxed);
      old_tail->next_ = node;
      while(!tail_.compare_exchange_strong(old_tail, old_tail->next_, std::memory_order_relaxed))
      {
        old_tail->next_ = node;
      }
      counter_.fetch_add(1, std::memory_order_release);
    }


    template<typename...Types>
    Node* push_back(Types&&...args)
    {
      Node* node = create_node(std::forward<Types>(args)...);
      push_back(node);
      return node;
    }


    void reset() noexcept
    {
      head_.store(nullptr, std::memory_order_relaxed);
      tail_.store(nullptr, std::memory_order_relaxed);
    }


    void increment
    (std::memory_order m = std::memory_order_release) noexcept
    {
      counter_.fetch_add(1, m);
    }


    void decrement
    (std::memory_order m = std::memory_order_release) noexcept 
    {
      counter_.fetch_sub(1,m);
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

      int64_t counter  = counter_.load(std::memory_order_acquire);
      if(counter <0)
      {
        PRINT("\n!!!!COUNTER LESS THAN ZERO!!!!\n");
        return 0;
      }
      return counter;
    }
};

template<typename T>
FreeList(T) -> FreeList<T>;
