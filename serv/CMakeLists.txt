cmake_minimum_required(VERSION 3.20)

project(server)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE flags)

add_executable(serv serv.cpp)

target_link_libraries(serv PRIVATE ${PROJECT_NAME})