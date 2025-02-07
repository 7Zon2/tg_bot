#pragma once
#include <memory_resource>

struct ShareResource
{
    static inline std::pmr::synchronized_pool_resource res_;
};
