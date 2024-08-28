#pragma once
#include <concepts>


namespace Pars
{
    namespace TG
    {
        class User;
        class chat;
        class message;
        class deletewebhook;
        class SetWebHook;
        class MessageOrigin;

        template<typename T>
        concept is_chat   = std::is_same_v<std::remove_reference_t<T>, chat>;

        template<typename T>
        concept is_user    = std::is_same_v<std::remove_reference_t<T>, User>;

        template<typename T>
        concept is_message = std::is_same_v<std::remove_reference_t<T>, message>;

        template<typename T>
        concept is_messageOriginBase = std::is_base_of_v<MessageOrigin, std::remove_reference_t<T>>; 
    }
}