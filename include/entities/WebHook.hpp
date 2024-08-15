
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

            public:

            WebhookInfo() {}

            WebhookInfo
            (
                json::string_view url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstrw ip_address         = {},
                optint  last_error_date    = {},
                optstrw last_error_message = {},
                optint last_synchronization_error_date         = {},
                optint max_connections                         = {},
                std::optional<std::vector<json::string>> allowed_updates = {}  
            )
            :
                url(url),
                has_custom_certificate(has_custom_certificate),
                pending_update_count(pending_update_count),
                ip_address(ip_address),
                last_error_date(last_error_date),
                last_error_message(last_error_message),
                last_synchronization_error_date(last_synchronization_error_date),
                max_connections(max_connections),
                allowed_updates(std::move(allowed_updates))
            {

            }

            private:

            [[nodiscard]]
            json::value
            fields_to_value() 
            {
                return TelegramRequestes::get_webhook_request
                (
                    url,
                    has_custom_certificate,
                    pending_update_count,
                    ip_address,
                    last_error_date,
                    last_error_message,
                    last_synchronization_error_date,
                    max_connections,
                    allowed_updates
                );
            }
            
        };

    }//namespace TG

}//namespace Pars
