#pragma once
#include <concepts>


namespace Pars
{
    namespace TG
    {
        struct User;
        struct chat;
        struct message;
        struct deletewebhook;
        struct SetWebHook;

        template<typename T>
        struct MessageOrigin;
        
        struct TelegramResponse;
        struct LinkPreviewOptions;
        struct PhotoSize;

        template<typename T>
        concept is_TelegramResponse = std::is_same_v<std::remove_reference_t<T>, TelegramResponse>;

        template<typename T>
        concept is_chat   = std::is_same_v<std::remove_reference_t<T>, chat>;

        template<typename T>
        concept is_user    = std::is_same_v<std::remove_reference_t<T>, User>;

        template<typename T>
        concept is_message = std::is_same_v<std::remove_reference_t<T>, message>;

        template<typename T>
        concept is_messageOriginBase = std::is_base_of_v<MessageOrigin<std::remove_reference_t<T>>, std::remove_reference_t<T>>; 

        template<typename T>
        concept is_PhotoSize = std::is_same_v<std::remove_reference_t<T>, PhotoSize>;

        template<typename T>
        concept is_LinkPreviewOptions = std::is_same_v<std::remove_reference_t<T>, LinkPreviewOptions>;
    }
}