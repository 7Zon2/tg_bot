#pragma once
#include "TelegramEntities.hpp"
#include "TelegramResponse.hpp"

namespace Pars
{
    namespace TG
    {
        struct deletewebhook : TelegramEntities<deletewebhook>
        {
            public:

            bool drop_pending_updates;

            static inline size_t req_fields = 1;
            static inline size_t opt_fields = 0;

            public:

            deletewebhook(){}

            deletewebhook(bool drop_pending_updates)
            :drop_pending_updates(drop_pending_updates){}


            [[nodiscard]]
            static 
            opt_fields_map 
            requested_fields(json::value val)
            {
               return TelegramResponse::verify_fields(std::move(val));
            }


            [[nodiscard]]
            static 
            std::unordered_map<json::string, json::value>
            optional_fields(json::value val)
            {
                return {};
            }


            template<is_fields_map T>
            void fields_from_map
            (T&& map)
            {
                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), std::make_pair("drop_pending_updates", std::ref(drop_pending_updates)));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::deletewebhook
                (
                    drop_pending_updates
                );
            }
        };
    }
}

