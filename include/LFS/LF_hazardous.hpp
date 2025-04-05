#pragma once
#include "LFS/share_resource.hpp"
#include "LF_FreeList.hpp"
#include "print.hpp"
#include <atomic>
#include <cstddef>
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
    ptr = reinterpret_cast<T*>(reinterpret_cast<size_t>(ptr) & (~bit_));

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

  FreeList<void*, true> rlist_;
  FreeList<HZP, true> hlist_;
  std::atomic<bool> isBusy = false;
  
  private:

  struct HZP
  {
    public:
    
    friend Hazardous;
    std::atomic<void*>  data_ = nullptr;
    std::atomic<size_t> data_size_ = 0;

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

    void clear() noexcept
    {
      data_.store(nullptr,std::memory_order_relaxed);
      data_size_.store(0, std::memory_order_relaxed);
    }


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

    friend Hazardous;
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

    hazard_pointer& operator = (std::nullptr_t) noexcept 
    {
      ptr_ = nullptr;
      return *this;
    }

    hazard_pointer(std::nullptr_t) noexcept:
      ptr_(nullptr){}

    hazard_pointer(hazard_node* ptr) noexcept:
      ptr_(ptr){}

    hazard_pointer() noexcept {}

    public:

    void reclaim() noexcept
    {
      assert(ptr_);
      auto next = ptr_->next();
      assert(is_tagged(next,2) == false);
      next = corrupt_node(next, 1);
      ptr_->next_.store(next, std::memory_order_release);
    }

    hazard_node* get()
    {
      return ptr_;
    }

    public:

    bool friend operator == (const hazard_pointer&, const hazard_pointer&) noexcept = default;
    
    bool operator ==(std::nullptr_t) noexcept
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
        assert(is_tagged(next,1) == false);
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

  void reclaim(hazard_pointer&& p)
  {
    p.reclaim();
    rlist_.push(p.ptr_);
    p = nullptr;
    check_limit();
  }


  void clear() noexcept
  {
    auto * node = hlist_.data();
    while(node)
    {
      node = clear_node_tag(node);
      auto* next = node->next();
      delete_hazard(node);
      node = next;
    }

    rlist_.clear();
    hlist_ = FreeList<HZP, true>{};
  }


  [[nodiscard]]
  hazard_node*
  find_free_node() noexcept
  {
    auto h = hlist_.data();
    auto t = hlist_.back();

    assert(h && t);

    while(h!=t)
    {
      auto next = h->next();
      auto pa = clear_tag(next, 2);
      bool is_not_active = pa.first;
      auto cleared = clear_node_tag(next);

      if(is_not_active) // try to make node is active if it's not
      {
        if(h->next_.compare_exchange_strong(next, cleared))
        {
          h->data_.clear();
          return h;
        }
      }
      h = cleared;
    }
    return nullptr;
  }


  [[nodiscard]]
  auto make_hazard() // all hazard's state is set here
  {
    hazard_node * next{};
    next = find_free_node();
    if(next)
    {
      PRINT_2("\nfree hazard\n");
      return hazard_pointer(next);   
    }

    PRINT_2("\nallocated hazard\n");

    auto back = hlist_.back();
    auto& tail = hlist_.tail();
   
    next = hlist_.create_node();
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
    std::pmr::vector<hazard_pointer> vec(&ShareResource::res_);
    vec.reserve(sz);
    for(size_t i = 0; i< sz; i++)
    {
      auto p = make_hazard();
      vec[i] = std::move(p);
    }
    return vec;
  }

  private:

  [[nodiscard]]
  hazard_node* 
  clear_node_tag(hazard_node* node) noexcept 
  {
    node = clear_tag(node, 1).second;
    node = clear_tag(node, 2).second;
    return node;
  }


  void 
  check_limit()
  { 
    size_t counter = rlist_.size();
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
    ShareResource::res_.deallocate(node, sizeof(hazard_node));
  }


  void 
  scan()
  {
    auto clear_next = [this](hazard_node* node)
    {
      auto next = node->next();
      next = clear_node_tag(next);
      node->next_.store(next, std::memory_order_release);
    };

    std::pmr::unordered_multimap <void*,hazard_node*> pmap{&ShareResource::res_}; //all not retired nodes
    std::pmr::unordered_map<void*, hazard_node*> rmap{&ShareResource::res_}; // retired nodes 

    for(auto r_node = rlist_.pop(); r_node; r_node = rlist_.pop())
    {
      void* ptr = r_node->data_;
      ShareResource::res_.deallocate(r_node, sizeof(decltype(*r_node)));

      if(!ptr)
        continue;

      hazard_node* node = static_cast<hazard_node*>(ptr);
      void* data = node->data_.data_.load(std::memory_order_relaxed);
      
      if(!data)
      {
        clear_next(node);
        continue;
      }

      auto it = rmap.insert({data, node});
      assert(it.second); // unfortunately this is true)
    }
 

    auto h = hlist_.data();
    auto t = hlist_.back();
    assert(t && h);
    hazard_node* next{};
    while(h!=t && h)
    {
      next = h->next();
      
      bool marked = is_tagged(next, 1);
      marked |= is_tagged(next, 2);

      if(!marked)
      {
        void* data = h->data_.data_.load(std::memory_order_relaxed);
        pmap.insert({data, h});
      }

      next = clear_node_tag(next); //clear to go on
      h = next;
    }



    PRINT("\n global hazard list size:",hlist_.size(),"\n");
    PRINT("\n pmap:", pmap.size(),"\n");
    PRINT("\n rmap:", rmap.size(),"\n");


    for(auto&& i : rmap) 
    {
      void * data = i.first;
      assert(data);

      if(!pmap.contains(data))
      {
        HZP& hzp = i.second->data_;
        size_t data_size = hzp.data_size_.load(std::memory_order_relaxed);
        deleter_(data, data_size);

        hazard_node * node = i.second;
        hazard_node* next = node->next(std::memory_order_acquire); 

        bool is_retired = is_tagged(next, 1);
        bool is_not_active = is_tagged(next, 2);
        assert(is_retired && (is_not_active == false));

        next = clear_tag(next, 1).second;
        next = corrupt_node(next, 2); // make node is not active
        node->next_.store(next, std::memory_order_release);
      }
      else
      {
        hazard_node* node = i.second;
        rlist_.push(node);
      }
    }

    isBusy.store(false, std::memory_order_release); //another thread after that store may call the scan method
  }
};
