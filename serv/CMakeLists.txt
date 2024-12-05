cmake_minimum_required(VERSION 3.20)

project(server)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(LD_LIBRARY_PATH "/usr/bin/ld/")
set(CMAKE_BUILD_TYPE Debug)

target_compile_features(flags INTERFACE cxx_std_23)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    target_compile_options(flags INTERFACE -pthread; -Wall; -O0; -g; -pedantic-errors;)
    target_link_options(flags INTERFACE -fsanitize=address)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Release)
    target_compile_options(flags INTERFACE -pthread; -Wall; -O2;)
endif()

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(serv serv.cpp)

target_link_libraries(serv PRIVATE ${PROJECT_NAME} )
