#pragma once
#include "LFS/share_resource.hpp"
#include "LF_FreeList.hpp"
#include "print.hpp"
#include <atomic>
#include <exception>
#include <functional>
#include <type_traits>
#include <unordered_map>


class Hazardous final  
{
  struct HZP;
  using del_foo  = std::function<void(void*, size_t)>;
  using hazard_node = typename FreeList<HZP>::Node;

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
      data_.store(data, std::memory_order_release);
      data_size_.store(data_size, std::memory_order_release);
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

    
    void reset() noexcept 
    {
      clear();
      isRetire.store(true, std::memory_order_relaxed);
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


    [[nodiscard]]
    void* 
    get() noexcept 
    {
      return data_.load(std::memory_order_relaxed);
    }

  };


  public:

  using HZP = HZP;

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
    for(auto* node = hlist_.pop(); node!=nullptr; node = hlist_.pop())
    {
      HZP& hzp = node->data_;
      if(hzp.isRetire)
        deleter_(hzp.data_, hzp.data_size_);
    }
    hlist_.clear();
  }


  void retire(HZP * hzp)
  {
    hzp->isRetire.store(true, std::memory_order_release);
  }


  [[nodiscard]]
  HZP* make_hazard()
  {
    check_limit();
    HZP hzp;
    return &hlist_.push(std::move(hzp))->data_;
  }


  [[nodiscard]]
  auto
  make_hazards(const size_t sz)
  {
    std::pmr::vector<HZP*> vec(sz, &ShareResource::res_);
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
    PRINT("deleted hazard node: ", node,"\n");
    ShareResource::res_.deallocate(node, sizeof(hazard_node));
  }


  void 
  scan()
  {
    std::pmr::unordered_multimap <void*,hazard_node*> pmap{&ShareResource::res_};
    std::pmr::unordered_multimap<void*, hazard_node*> rmap{&ShareResource::res_};

    FreeList<HZP> hlist{std::move(hlist_)};
    size_t size = hlist.size(); 
    pmap.reserve(size);
    rmap.reserve(size);
    PRINT("SCAN\nSize of the hazard pointers: ", hlist.size(),"\n");
    for(auto* pn = hlist.pop(); pn!=nullptr; pn = hlist.pop())
    {
      HZP& hzp = pn->data_;
      void * data = hzp.data_.load(std::memory_order_acquire);
      bool is_retire = hzp.isRetire.load(std::memory_order_acquire);
      if(!is_retire)
      {
        hlist_.push(pn);
        pmap.emplace(data,pn);
      }
      else
      {
        if(data)
          rmap.emplace(data, pn);
      }  
    }

    isBusy.store(false, std::memory_order_relaxed);

    for(auto&& i : rmap)
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


