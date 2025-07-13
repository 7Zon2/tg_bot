
#pragma once 
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct WebhookInfo : TelegramEntities<WebhookInfo>
        {
            public:

            json::string url;
            bool has_custom_certificate;
            uint64_t pending_update_count;
            optstrw ip_address         = {};
            optint  last_error_date    = {};
            optstrw last_error_message = {};
            optint last_synchronization_error_date         = {};         
            optint max_connections                         = {};
            std::optional<std::vector<json::string>> allowed_updates = {}; 


            static inline size_t req_fields = 3;
            static inline size_t opt_fields = 6;

            public:

            WebhookInfo() {}

            WebhookInfo
            (
                json::string url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstr ip_address         = {},
                optint  last_error_date    = {},
                optstr last_error_message = {},
                optint last_synchronization_error_date         = {},
                optint max_connections                         = {},
                std::optional<std::vector<json::string>> allowed_updates = {}  
            )
            :
                url(std::move(url)),
                has_custom_certificate(has_custom_certificate),
                pending_update_count(pending_update_count),
                ip_address(std::move(ip_address)),
                last_error_date(last_error_date),
                last_error_message(std::move(last_error_message)),
                last_synchronization_error_date(last_synchronization_error_date),
                max_connections(max_connections),
                allowed_updates(std::move(allowed_updates))
            {

            }
            
        };

    }//namespace TG

}//namespace Pars
