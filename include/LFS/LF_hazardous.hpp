#pragma once
#include "LF_FreeList.hpp"
#include <exception>
#include <functional>
#include <unordered_map>

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


