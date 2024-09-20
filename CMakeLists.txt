cmake_minimum_required(VERSION 3.20)

project(http)

set(BOOST_ROOT /home/zon/my_libs/boost_1_86_0)

find_package(Boost REQUIRED)

find_package(Boost REQUIRED COMPONENTS json)

find_package(OpenSSL REQUIRED)

add_library(flags INTERFACE)

include_directories(${Boost_INCLUDE_DIRS})

target_compile_features(flags INTERFACE cxx_std_20)

target_compile_options(flags INTERFACE -pthread; -Wall; -O0; -g)

target_include_directories(flags INTERFACE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(flags INTERFACE  OpenSSL::SSL OpenSSL::Crypto ${Boost_LIBRARIES})

add_subdirectory(serv)
add_subdirectory(client)
