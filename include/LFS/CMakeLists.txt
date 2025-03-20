cmake_minimum_required(VERSION 3.20)

project(lfs)

add_library( LFS INTERFACE
    LF_stack.hpp
    LF_allocator.hpp
    LF_hazardous.hpp
    LF_FreeList.hpp
    LF_HashTable.hpp
    share_resource.hpp
)
