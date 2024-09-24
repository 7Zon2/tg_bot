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
                json::string url,
                optstr certificate = {},
                optstr ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstr  secret_token         = {}
            )
            :
                url(std::move(url)),
                certificate(std::move(certificate)),
                ip_address(std::move(ip_address)),
                max_connections(max_connections),
                allowed_updates(std::move(allowed_updates)),
                drop_pending_updates(drop_pending_updates),
                secret_token(std::move(secret_token))
            {
                
            }

            public:


            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val) 
            {
                const size_t sz = 1;

                auto opt = MainParser::check_pointer_validation(std::move(val), std::make_pair("/setwebhook", json::kind::object));
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


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self) 
            {
                return TelegramRequestes::setWebhook
                (
                    forward_like<Self>(url),
                    forward_like<Self>(certificate),
                    forward_like<Self>(ip_address),
                    max_connections,
                    forward_like<Self>(allowed_updates),
                    drop_pending_updates,
                    forward_like<Self>(secret_token)
                );
            }
        };
        
    }//namespace TG

}//namespace Pars
