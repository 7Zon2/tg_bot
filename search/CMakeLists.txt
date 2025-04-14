cmake_minimum_required(VERSION 3.20)

project(search)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

target_include_directories(flags INTERFACE ../../include)

add_executable(srch search.cpp)

target_link_libraries(srch PRIVATE ${PROJECT_NAME})
