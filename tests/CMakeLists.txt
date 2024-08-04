cmake_minimum_required(VERSION 3.20)

project(tests)

find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS json)

add_library(flags INTERFACE)

include_directories(${Boost_INCLUDE_DIRS})

target_compile_features(flags INTERFACE cxx_std_20)

target_compile_options(flags INTERFACE -pthread; -Wall; -O0; -g)

target_include_directories(flags INTERFACE ../include)

add_executable(main tg_req.cpp)

target_link_libraries(flags INTERFACE ${Boost_LIBRARIES})

target_link_libraries(main PRIVATE flags)
