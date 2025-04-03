#pragma once
#include "LFS/share_resource.hpp"
#include "LF_hazardous.hpp"
#include <atomic>
#include <type_traits>

class LF_allocator : public std::pmr::memory_resource
{
    protected:

    std::atomic<size_t> limit_;
    std::function<void(void*, size_t)> custom_deleter_; 
    Hazardous hazardous_;

    using default_deleter = decltype([](void* data, size_t sz){
            ShareResource::res_.deallocate(data, sz);
        });

    public:

    LF_allocator
    (
     std::function<void(void*, size_t)>deleter = default_deleter{}, 
     const size_t limit = 10
    ):
      limit_(limit),
      custom_deleter_
      (
        [del = deleter](void * data, size_t data_size)
        {
          if(data)
          {
            using type = std::remove_cvref_t<decltype(del)>;
            if constexpr(std::is_same_v<type, default_deleter>)
            {
              del(data, data_size);
            }
            else
            {
              del(data, data_size);
              ShareResource::res_.deallocate(data, data_size);
            }
          }
        }
      ), 
      hazardous_
      (
       limit,
       custom_deleter_
      )
    {}


    LF_allocator(const LF_allocator&) = delete;
    LF_allocator& operator = (const LF_allocator&) = delete;


    LF_allocator(LF_allocator&& rhs) noexcept:
      limit_(rhs.limit_.load(std::memory_order_relaxed)),
      custom_deleter_(rhs.custom_deleter_),
      hazardous_(std::move(rhs.hazardous_))
    {}

    
    LF_allocator& operator = (LF_allocator&& rhs) noexcept
    {
      if(this!=&rhs)
      {
        limit_.store(rhs.limit_, std::memory_order_relaxed);
        custom_deleter_ = rhs.custom_deleter_;
        hazardous_ = std::move(rhs.hazardous_);
      }
      return *this;
    }


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

    void clear()
    {
      hazardous_.clear();
    }


    [[nodiscard]]
    auto get_hazard()
    {
      return hazardous_.make_hazard();
    }


    void reclaim_hazard(Hazardous::hazard_pointer&& p)
    {
      hazardous_.reclaim(std::move(p));
    }


    void 
    set_retire_limit
    (const size_t limit) noexcept
    {
      limit_.store(limit, std::memory_order_release);
    }


    [[nodiscard]]
    size_t 
    get_retire_limit() const noexcept
    {
      return limit_.load(std::memory_order_acquire);
    }
};
