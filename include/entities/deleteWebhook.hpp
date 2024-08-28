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

            deletewebhook(){}

            deletewebhook(bool drop_pending_updates)
            :drop_pending_updates(drop_pending_updates){}


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map 
            requested_fields(T&& val)
            {
               return TelegramResponse::verify_fields(std::forward<T>(val));
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            std::unordered_map<json::string, json::value>
            optional_fields(T&& val)
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


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::deletewebhook
                (
                    drop_pending_updates
                );
            }
        };
    }
}

