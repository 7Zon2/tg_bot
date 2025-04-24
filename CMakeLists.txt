cmake_minimum_required(VERSION 3.20)

project(http)

set(BOOST_ROOT "/home/zon/my_libs/boost_1_86_0")
set(LEX_ROOT "/usr/local/lib64/liblexbor.so.2.5.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS json)
find_package(Boost REQUIRED COMPONENTS url)
find_package(Boost REQUIRED COMPONENTS iostreams)
find_package(OpenSSL REQUIRED)
find_package(LEX_LIBRARY HINTS ${LEX_ROOT})

add_library(flags INTERFACE)

target_include_directories(flags INTERFACE ${BOOST_ROOT})
target_include_directories(flags INTERFACE "/usr/local/lib64/")

target_compile_features(flags INTERFACE cxx_std_23)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    target_compile_options(flags INTERFACE -pthread; -Wall; -O0; -g; -pedantic-errors;)
    target_link_options(flags INTERFACE -fsanitize=address)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Release)
    target_compile_options(flags INTERFACE -pthread; -Wall; -O2;)
endif()

target_include_directories(flags INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(flags INTERFACE  OpenSSL::SSL OpenSSL::Crypto  stdc++exp dl  ${Boost_LIBRARIES} Boost::json Boost::url Boost::iostreams ${LEX_ROOT})

add_subdirectory(client)
