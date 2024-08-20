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

            [[nodiscard]]
            static
            std::optional<std::unordered_map<json::string, json::value>> 
            requested_fields(const json::value& val) 
            {
                const size_t sz = 1;

                auto opt = MainParser::check_pointer_validation(val, std::make_pair("/setwebhook", json::kind::object));
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
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/setwebhook/certificate", json::kind::string),
                    std::make_pair("/setwebhook/ip_address", json::kind::string),
                    std::make_pair("/setwebhook/max_connections", json::kind::int64),
                    std::make_pair("/setwebhook/allowed_updates", json::kind::array),
                    std::make_pair("/setwebhook/drop_pending_updates", json::kind::bool_),
                    std::make_pair("/setwebhook/secret_token", json::kind::string)
                );

                return map;
            } 


            void 
            fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("url", std::ref(url)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("certificate", std::ref(certificate)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("ip_address", std::ref(ip_address)));

                MainParser::field_from_map
                <json::kind::int64>(map, std::make_pair("max_connections", std::ref(max_connections)));

                MainParser::field_from_map
                <json::kind::array>(map, std::make_pair("allowed_updates", std::ref(allowed_updates)));

                MainParser::field_from_map
                <json::kind::bool_>(map, std::make_pair("drop_pending_updates", std::ref(drop_pending_updates)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("secret_token", std::ref(secret_token)));
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
