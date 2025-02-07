#pragma once
#include "boost/core/span.hpp"
#include "print.hpp"
#include <exception>
#include <functional>
#include <memory>
#include <atomic>
#include <memory_resource>
#include <cassert>
#include <thread>
#include <algorithm>

namespace pmr = std::pmr;

struct ShareResource
{
    static inline std::pmr::synchronized_pool_resource res_;
};

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



class Hazardous final : ShareResource 
{
  struct HZP;
  using del_foo  = std::function<void(void*, size_t)>;
  using hazard_node = typename FreeList<HZP>::node_type;

  del_foo deleter_;
  size_t limit_;

  FreeList<HZP> hlist_;
  std::atomic<bool> isBusy = false;
  
  private:

  struct HZP
  {
    private:
    
    friend Hazardous;
    std::atomic<void*>  data_ = nullptr;
    std::atomic<bool>   isRetire = false;
    std::atomic<size_t> data_size_ = 0;

    public:
    
    HZP() noexcept = default;


    HZP(HZP&& rhs) noexcept
    {
      rhs.isRetire = isRetire.exchange(rhs.isRetire, std::memory_order_relaxed);
      rhs.data_size_ = data_size_.exchange(rhs.data_size_, std::memory_order_relaxed);
      rhs.data_ = data_.exchange(rhs.data_, std::memory_order_relaxed);
    }


    HZP& operator = (HZP&& rhs) noexcept
    {
      if(&rhs != this)
      {
        rhs.isRetire = isRetire.exchange(rhs.isRetire, std::memory_order_relaxed);
        rhs.data_size_ = data_size_.exchange(rhs.data_size_, std::memory_order_relaxed);
        rhs.data_ = data_.exchange(rhs.data_,std::memory_order_relaxed); 
      }
      return  *this;
    }


    void protect(void * data, const size_t data_size = 0) noexcept
    {
      data_.store(data, std::memory_order_release);
      data_size_.store(data_size, std::memory_order_release);
    }


    template<typename T>
    void protect(T& data) noexcept
    {
      size_t data_size = sizeof(T);
      protect(static_cast<void*>(&data), data_size);
    }

    
    void clear() noexcept
    {
      data_.store(nullptr, std::memory_order_release);
      data_size_.store(0, std::memory_order_release);
    }


    [[nodiscard]]
    size_t 
    get_data_size() const noexcept
    {
      return data_size_.load(std::memory_order_acquire);
    }

  };


  public:

  using HZP = HZP;

  Hazardous(const size_t limit, del_foo deleter):
    deleter_(deleter), limit_(limit){}


  ~Hazardous()
  {
    
  }

  public:

  void retire(HZP * hzp)
  {
    hzp->isRetire.store(true, std::memory_order_release);
  }


  [[nodiscard]]
  HZP* make_hazard()
  {
    check_limit();
    HZP hzp;
    return &hlist_.push(std::move(hzp))->get();
  }


  [[nodiscard]]
  auto
  make_hazards(const size_t sz)
  {
    std::pmr::vector<HZP*> vec(sz, &res_);
    for(size_t i = 0; i< sz; i++)
    {
      HZP* p = make_hazard();
      vec[i] = p;
    }
    return vec;
  }

  private:

  void 
  check_limit()
  { 
    size_t counter = hlist_.size();
    if(counter >= limit_)
    {
      bool old = isBusy.exchange(true, std::memory_order_relaxed);
      if(!old)
      {
        try
        {
          scan();
        }
        catch(const std::exception& ex) 
        {
          isBusy.store(false, std::memory_order_relaxed);
          auto ptr = std::current_exception();
          std::rethrow_exception(ptr);
        }
      }
    }
  }


  void 
  delete_hazard(hazard_node* node) noexcept
  {
    ShareResource::res_.deallocate(node, sizeof(hazard_node));
  }


  void 
  scan()
  {
    std::pmr::unordered_multimap <void*,hazard_node*> pmap{&res_};
    std::pmr::unordered_multimap<void*, hazard_node*> rmap{&res_};

    FreeList<HZP> hlist{std::move(hlist_)};
    size_t size = hlist.size(); 
    pmap.reserve(size);
    rmap.reserve(size);

    for(auto* pn = hlist.pop(); pn!=nullptr; pn = hlist.pop())
    {
      HZP& hzp = pn->get();
      void * data = hzp.data_.load(std::memory_order_acquire);
      bool is_retire = hzp.isRetire.load(std::memory_order_acquire);
      if(!is_retire)
      {
        hlist_.push(pn);
        pmap.emplace(data,pn);
      }
      else
      {
        rmap.emplace(data, pn);
      }  
    }

    isBusy.store(false, std::memory_order_relaxed);

    for(auto&& i : rmap)
    {
      void * data = i.first;
      if(!data)
      {
        continue;
      }

      if(!pmap.contains(data))
      {
        size_t data_size = i.second->get().data_size_.load(std::memory_order_acquire);
        deleter_(data, data_size);
        auto iters = rmap.equal_range(data);
        auto b = iters.first;
        auto e = iters.second;
        for(;b!=e;b++)
        {
          auto* node = b->second;
          delete_hazard(node);
        }
      }
    }
  }
};


class LF_allocator : public pmr::memory_resource, ShareResource
{
    protected:

    std::atomic<size_t> limit_;
    std::function<void(void*, size_t)> custom_deleter_; 
    Hazardous hazardous_;

    public:

    LF_allocator(std::function<void(void*, size_t)>deleter, const size_t limit = 100):
      limit_(limit),
      custom_deleter_
      (
        [del = deleter](void * data, size_t data_size)
        {
          del(data, data_size);
          ShareResource::res_.deallocate(data, data_size);
        }
      ), 
      hazardous_
      (
       limit,
       custom_deleter_
      )
    {
    }

    LF_allocator(const LF_allocator&) = delete;
    LF_allocator(LF_allocator&&) = delete;
    LF_allocator& operator = (const LF_allocator&) = delete;
    LF_allocator& operator = (LF_allocator&&) = delete;

    virtual 
    ~LF_allocator()
    {

    }

    protected:

    virtual void*
    do_allocate(size_t bytes, size_t alignment) override
    {
        void *  p = ShareResource::res_.allocate(bytes, alignment);
        return p;
    }

    virtual void
    do_deallocate(void* p, size_t bytes, size_t alignment) override
    {
      custom_deleter_(p, bytes);
    }

    virtual bool
    do_is_equal(const memory_resource& other) const noexcept override
    {
        return false;
    }

    public:

    void deallocate_hazard(Hazardous::HZP* hzp)
    {
      hazardous_.retire(hzp);
    }


    [[nodiscard]]
    Hazardous::HZP* 
    get_hazard()
    {
      auto * hzp = hazardous_.make_hazard();
      return hzp;
    }


    void 
    set_retire_limit
    (const size_t limit) noexcept
    {
      limit_.store(limit, std::memory_order_release);
    }

    size_t 
    get_retire_limit() const noexcept
    {
      return limit_.load(std::memory_order_acquire);
    }
};


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
