cmake_minimum_required(VERSION 3.20)

project(client)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(cl client.cpp)

target_link_libraries(cl PRIVATE ${PROJECT_NAME})