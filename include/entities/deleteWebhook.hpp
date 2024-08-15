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



            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>> 
            requested_fields(const json::value& val)
            {
               return TelegramResponse::verify_fields(val);
            }


            [[nodiscard]]
            static 
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return {};
            }


            void fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::bool_>(map, std::make_pair("drop_pending_updates", std::ref(drop_pending_updates)));
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

