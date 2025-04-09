cmake_minimum_required(VERSION 3.20)

project(search)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(search search.cpp)

target_link_libraries(search PRIVATE ${PROJECT_NAME})

