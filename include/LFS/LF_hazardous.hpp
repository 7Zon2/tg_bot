#pragma once
#include "LFS/share_resource.hpp"
#include "LF_FreeList.hpp"
#include "print.hpp"
#include <atomic>
#include <exception>
#include <functional>
#include <type_traits>


[[nodiscard]]
inline size_t 
pointer_cast(void* ptr) noexcept
{
  return reinterpret_cast<size_t>(ptr);
}


template<typename T = void*>
[[nodiscard]]
inline auto
pointer_cast(size_t  ptr) noexcept 
{
  using type = std::add_pointer_t<std::remove_pointer_t<T>>;
  return reinterpret_cast<type>(ptr);
}


template<typename T>
[[nodiscard]]
inline static 
T* corrupt_node
(T * ptr, size_t bit = 1) noexcept
{
  size_t bit_ = 1 << (bit - 1);
  return reinterpret_cast<T*>(reinterpret_cast<size_t>(ptr) | bit_);
}


template<typename T>
[[nodiscard]]
static inline 
bool is_tagged 
(T* ptr, size_t bit = 1)  noexcept 
{
  size_t bit_ = 1 << (bit-1);
  return reinterpret_cast<size_t>(ptr) & bit_; 
}


template<typename T>
[[nodiscard]]
inline static 
auto clear_tag
(T* ptr, size_t bit = 1) noexcept
{
  size_t bit_ = 1 << (bit-1);
  bool Tagged = is_tagged(ptr, bit);
  if(Tagged)
    ptr = reinterpret_cast<T*>(reinterpret_cast<size_t>(ptr) ^ bit_);

  return std::make_pair(Tagged, ptr);
}


template<typename T>
[[nodiscard]]
inline static
auto concat_ptr(void* lhs, void* rhs) noexcept 
{
  size_t l = pointer_cast(lhs);
  size_t r = pointer_cast(rhs);
  size_t m = l | r;
  return pointer_cast<T>(m);
}



class Hazardous final  
{
  struct HZP;
  using del_foo  = std::function<void(void*, size_t)>;
  using hazard_node = typename FreeList<HZP, true>::Node;

  del_foo deleter_;
  size_t limit_;

  FreeList<HZP, true> hlist_;
  std::atomic<bool> isBusy = false;
  
  private:

  struct HZP
  {
    public:
    
    friend Hazardous;
    std::atomic<void*>  data_ = nullptr;
    std::atomic<size_t> data_size_ = 0;
    std::atomic<void*> tag_{};

    public:
    
    HZP() noexcept = default;

    ~HZP()
    {}

    template<typename T>
    HZP(T& data) noexcept
    {
      protect(data);
    }
    

    template<typename T>
    HZP& operator = (T& data) noexcept
    {
      protect(data);
      return *this;
    }

    public:

    void protect(void * data, const size_t data_size = 0) noexcept
    {
      data_.store(data, std::memory_order_relaxed);
      data_size_.store(data_size, std::memory_order_relaxed);
      std::atomic_thread_fence(std::memory_order_release);
    }


    template<typename T>
    void protect(T& data) noexcept
    {
      size_t data_size = 0;
      void * ptr{};
      if constexpr (std::is_pointer_v<T>)
      {
        data_size = sizeof(std::remove_pointer_t<T>);
        ptr = static_cast<void*>(&*data);
      }
      else
      {
        data_size = sizeof(T);
        ptr = static_cast<void*>(&data);
      }
      protect(ptr, data_size);
    }
  };


  public:

  using HZP = HZP;
  
  struct hazard_pointer
  {
    private:

    hazard_node* ptr_{};

    public:

    hazard_pointer(const hazard_pointer&) = delete;
    hazard_pointer& operator == (const hazard_pointer&) = delete;

    hazard_pointer(hazard_pointer&& rhs) noexcept
    {
      std::swap(ptr_, rhs.ptr_);
    }

    hazard_pointer& operator = (hazard_pointer&& rhs) noexcept
    {
      if(this!=&rhs)
      {
        std::swap(ptr_,rhs.ptr_);
      }
      return *this;
    }

    hazard_pointer(hazard_node* ptr):
      ptr_(ptr){}

    hazard_pointer(){}

    hazard_pointer(std::nullptr_t):
      ptr_(nullptr){}

    public:

    void reclaim() noexcept
    {
      auto next = ptr_->next();
      next = corrupt_node(next, 1);
      ptr_->next_.store(next, std::memory_order_release);
      ptr_ = nullptr;
    }

    public:

    bool friend operator == (const hazard_pointer&, const hazard_pointer&) = default;
    
    bool operator ==(std::nullptr_t)
    {
      return ptr_ == nullptr;
    }

    HZP& operator *() const noexcept 
    {
      return ptr_->data_;
    }

    HZP* operator->() const noexcept
    {
      return &ptr_->data_;
    }

    operator bool() noexcept
    {
      return ptr_;
    }

    ~hazard_pointer()
    {
      if(ptr_)
      {
        auto next = ptr_->next();
        next = corrupt_node(next, 2);
        ptr_->next_.store(next, std::memory_order_release);
        ptr_ = nullptr;
      }
    }
  };

  public:

  Hazardous(const size_t limit,const del_foo& deleter):
    deleter_(deleter), limit_(limit){}

  Hazardous(const size_t limit, del_foo&& deleter) noexcept:
    deleter_(std::move(deleter)), limit_(limit){}

  Hazardous(const Hazardous&) = delete;
  Hazardous& operator = (const Hazardous&) = delete;

  Hazardous(Hazardous&& rhs) noexcept
  {
    limit_ = rhs.limit_;
    deleter_ = std::move(rhs.deleter_);
    bool busy = true; 
    do
    {
      busy = rhs.isBusy.load(std::memory_order_relaxed);
    }
    while(busy || !rhs.isBusy.compare_exchange_strong(busy, true, std::memory_order_relaxed));

    hlist_ = std::move(rhs.hlist_);
    rhs.isBusy.store(false, std::memory_order_release);
  }

  Hazardous& operator=(Hazardous&& rhs) noexcept
  {
    if(this!=&rhs)
    {
      Hazardous temp{std::move(rhs)};
      limit_ = temp.limit_;
      deleter_ = std::move(temp.deleter_);
      hlist_ = std::move(temp.hlist_);
    }
    return *this;
  }

  ~Hazardous()
  {
    
  }

  public:

  void clear()
  {
    auto * node = hlist_.data();
    while(node)
    {
      node = clear_tag(node,1).second;
      node = clear_tag(node,2).second;
      auto* next = node->next();
      delete_hazard(node);
      node = next;
    }
    
    hlist_ = FreeList<HZP, true>{};
  }


  [[nodiscard]]
  auto make_hazard() // all hazard's state is set here
  {
    check_limit();

    auto back = hlist_.back();
    auto& tail = hlist_.tail();
   
    hazard_node * next = hlist_.create_node();
    while(!tail.compare_exchange_strong(back, next, std::memory_order_release))
    {}
   
    hazard_node * bnext = back->next_.load(std::memory_order_relaxed); // assign next for tail while it's not changed 
    back->next_ = concat_ptr<hazard_node*>(next, bnext);
    hlist_.increment(std::memory_order_relaxed);
    return hazard_pointer{back};
  }


  [[nodiscard]]
  auto make_hazards(const size_t sz)
  {
    std::pmr::vector<hazard_pointer> vec(sz, &ShareResource::res_);
    for(size_t i = 0; i< sz; i++)
    {
      auto p = make_hazard();
      vec[i] = std::move(p);
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
      bool old = isBusy.exchange(true, std::memory_order_acquire);
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
  static delete_hazard(hazard_node* node) noexcept
  {
    PRINT("deleted hazard node: ", node,"\n");
    PRINT("delete thread id:", std::this_thread::get_id(),"\n");
    ShareResource::res_.deallocate(node, sizeof(hazard_node));
  }


  void 
  scan()
  {
    std::pmr::unordered_multimap <void*,hazard_node*> pmap{&ShareResource::res_};
    std::pmr::unordered_multimap<void*, hazard_node*> rmap{&ShareResource::res_};
    std::pmr::vector<hazard_node*> hvec{&ShareResource::res_};

    auto emplace = [&](hazard_node* pn, bool is_active, bool is_retire)
    {
      HZP& hzp = pn->data_;
      void * data = hzp.data_.load(std::memory_order_relaxed);

      if(!is_active)
      {
        //print("\nIsNotActive\n");
        assert(!is_retire);
        delete_hazard(pn);
        return;
      }
      
      if(!is_retire)
      {
        hvec.push_back(pn);
        if(data)
          pmap.emplace(data,pn);
      }
      else //the state in which pointers are no longer used
      {
        assert(data);
        rmap.emplace(data, pn);
      }
    };

  
    auto h = hlist_.data();
    auto t = hlist_.back();
    assert(t);
    auto& head = hlist_.head();
    hazard_node* next{};
    while(h!=t && h)
    {
      bool is_active = true;
      bool is_retire = false;
      next = h->next();
      
      auto pa = clear_tag(next, 1);
      bool marked = pa.first;
      if(marked)
      {
        next = pa.second;
        is_active = true;
        is_retire = true;
      }

      pa = clear_tag(next,2);
      marked = pa.first;
      if(marked)
      {
        is_active = false;
        is_retire = false;
      }


      next = pa.second; //store cleared next and then pop a head
      h->next_ = next;
      if(!head.compare_exchange_strong(h, next, std::memory_order_release))
      {
        assert(true);
      }
      hlist_.decrement();
      emplace(h, is_active, is_retire);
      h = hlist_.data();
    }

    for(auto pn: hvec)
      hlist_.push(pn);

    isBusy.store(false, std::memory_order_release); //another thread after that store may call the scan method

    PRINT("\n global hazard list size:",hlist_.size(),"\n");
    PRINT("\n pmap:", pmap.size(),"\n");
    PRINT("\n rmap:", rmap.size(),"\n");


    for(auto&& i : rmap) //maps must not have null nodes 
    {
      void * data = i.first;
      if(!pmap.contains(data))
      {
        HZP& hzp = i.second->data_;
        size_t data_size = hzp.data_size_.load(std::memory_order_relaxed);
        deleter_(data, data_size);

        auto iters = rmap.equal_range(data);
        auto b = iters.first;
        auto e = iters.second;
        for(;b!=e;b++)
        {
          PRINT("\ndelete HZP:",b->second,"\n");
          auto* node = b->second;
          delete_hazard(node);
        }
      }

      [[maybe_unused]]
      size_t remove_number = rmap.erase(data);
      PRINT("number of removed elements:", remove_number,"\n");
    }
  }
};
