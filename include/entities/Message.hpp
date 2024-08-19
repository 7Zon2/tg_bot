#pragma once 
#include "User.hpp"
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct message : TelegramEntities<message>
        {
            int64_t message_id;
            optint message_thread_id  = {};
            std::optional<User> from = {};
            //Chat
        };
    }
}