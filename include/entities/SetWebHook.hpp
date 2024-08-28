#pragma once
#include "TelegramEntities.hpp"


namespace Pars
{
    namespace TG
    {
        struct SetWebHook : TelegramEntities<SetWebHook>
        {
            public:

            json::string url;
            optstr certificate = {};
            optstr ip_address  = {};
            optint max_connections = {};
            std::optional<json::array> allowed_updates = {};
            optbool drop_pending_updates = {};
            optstr  secret_token         = {};

            public:

            SetWebHook(){}

            SetWebHook
            (
                json::string_view url,
                optstr certificate = {},
                optstr ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstr  secret_token         = {}
            )
            :
                url(url),
                certificate(certificate),
                ip_address(ip_address),
                max_connections(max_connections),
                allowed_updates(allowed_updates),
                drop_pending_updates(drop_pending_updates),
                secret_token(secret_token)
            {
                
            }

            public:


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val) 
            {
                const size_t sz = 1;

                auto opt = MainParser::check_pointer_validation(std::forward<T>(val), std::make_pair("/setwebhook", json::kind::object));
                if(opt.has_value() == false)
                {
                    return std::nullopt;
                }
                
                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/setwebhook/url", json::kind::uint64)
                );

                if(map.size() < sz)
                {
                    return std::nullopt;
                }

                return map;
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            fields_map
            optional_fields(T&& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair("/setwebhook/certificate", json::kind::string),
                    std::make_pair("/setwebhook/ip_address", json::kind::string),
                    std::make_pair("/setwebhook/max_connections", json::kind::int64),
                    std::make_pair("/setwebhook/allowed_updates", json::kind::array),
                    std::make_pair("/setwebhook/drop_pending_updates", json::kind::bool_),
                    std::make_pair("/setwebhook/secret_token", json::kind::string)
                );

                return map;
            } 


            template<is_fields_map T>
            void 
            fields_from_map
            (T&& map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(url));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(certificate));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(ip_address));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(max_connections));

                MainParser::field_from_map
                <json::kind::array>(std::forward<T>(map), MAKE_PAIR(allowed_updates));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(drop_pending_updates));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(secret_token));
            }


            [[nodiscard]]
            json::value
            fields_to_value() 
            {
                return TelegramRequestes::setWebhook
                (
                    url,
                    certificate,
                    ip_address,
                    max_connections,
                    allowed_updates,
                    drop_pending_updates,
                    secret_token
                );
            }
        };
        
    }//namespace TG

}//namespace Pars
