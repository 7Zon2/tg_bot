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
            //MessageOrigin
             
        };
    }
}