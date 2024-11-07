#pragma once
#include <concepts>


namespace Pars
{
    namespace TG
    {
        template<typename Derived>
        struct TelegramEntities;
        struct User;
        struct Chat;
        struct message;
        struct deletewebhook;
        struct SetWebHook;
        struct SendMessage;
        struct MessageOrigin;
        struct TelegramResponse;
        struct LinkPreviewOptions;
        struct PhotoSize;
        struct Animation;
        struct Audio;
        struct Document;
        struct Story;
        struct Video;
        struct VideoNote;
        struct Voice;

        template<typename T>
        concept is_TelegramBased = std::is_base_of_v<TelegramEntities<typename T::type>, typename T::type>;

        template<typename T>
        concept is_TelegramResponse = std::is_same_v<std::remove_reference_t<T>, TelegramResponse>;

        template<typename T>
        concept is_chat   = std::is_same_v<std::remove_reference_t<T>, Chat>;

        template<typename T>
        concept is_user    = std::is_same_v<std::remove_reference_t<T>, User>;

        template<typename T>
        concept is_message = std::is_same_v<std::remove_cvref_t<T>, message>;

        template<typename T>
        concept is_sendmessage = std::is_same_v<std::remove_reference_t<T>, SendMessage>;

        template<typename T>
        concept is_messageOriginBase = std::is_base_of_v<MessageOrigin, std::remove_reference_t<T>>; 

        template<typename T>
        concept is_PhotoSize = std::is_same_v<std::remove_reference_t<T>, PhotoSize>;

        template<typename T>
        concept is_LinkPreviewOptions = std::is_same_v<std::remove_reference_t<T>, LinkPreviewOptions>;

        template<typename T>
        concept is_Animation = std::is_same_v<std::remove_reference_t<T>, Animation>;

        template<typename T>
        concept is_Audio = std::is_same_v<std::remove_reference_t<T>, Audio>;

        template<typename T>
        concept is_Document = std::is_same_v<std::remove_reference_t<T>, Document>;

        template<typename T>
        concept is_Story = std::is_same_v<std::remove_reference_t<T>, Story>;

        template<typename T>
        concept is_Video = std::is_same_v<std::remove_reference_t<T>, Video>;

        // template<typename T>
        // concept VideoNote = std::is_same_v<std::remove_reference_t<T>, VideoNote>;

        // template<typename T>
        // concept Voice = std::is_same_v<std::remove_reference_t<T>, Voice>;
    }
}