#pragma once
#include "LF_hazardous.hpp"

class LF_allocator : public std::pmr::memory_resource
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

