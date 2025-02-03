cmake_minimum_required(VERSION 3.20)

project(headers)

add_library(Headers INTERFACE
    coro_future.hpp
    head.hpp
    certif.hpp
    json_head.hpp
    print.hpp
    tg_pars.hpp 
    move_opt.hpp
)

target_include_directories(Headers INTERFACE entities)
target_include_directories(Headers INTERFACE LFS)
