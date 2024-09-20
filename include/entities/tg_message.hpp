#pragma once 
#include "User.hpp"
#include "Chat.hpp"
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct message : TelegramEntities<message>
        {
            using optuser = std::optional<User>;
            using optchat = std::optional<chat>;

            int64_t message_id;
            uint64_t date;
            chat chat_;

            optint  message_thread_id;
            optuser from;
            optchat sender_chat;
            optint  sender_boost_count;
            optuser sender_business_bot;
            optstr  business_connection_id;
            std::optional<std::reference_wrapper<MessageOrigin>> forward_origin;
            optbool is_topic_message;
            optbool is_automatic_forward;
            std::optional<std::reference_wrapper<message>> reply_to_message;
             

             static inline size_t req_fields = 3;
             static inline size_t opt_fields = 10;
        };
    }
}