cmake_minimum_required(VERSION 3.20)

project(entities)

add_library( Entities INTERFACE
    TelegramEntities.hpp
    TelegramResponse.hpp
    SetWebHook.hpp
    User.hpp
    WebHook.hpp
    entities.hpp
)