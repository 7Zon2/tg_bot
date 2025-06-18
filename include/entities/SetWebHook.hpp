#pragma once
#include "TelegramEntities.hpp"


namespace Pars
{
    namespace TG
    {
        struct SetWebHook : TelegramEntities<SetWebHook>
        {
            using TelegramEntities::operator =;

            static const inline  json::string entity_name{"setwebhook"};
            static constexpr  size_t req_fields = 1;
            static constexpr  size_t opt_fields = 6;  

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

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val) 
            {

                auto opt = MainParser::check_pointer_validation(std::move(val), std::make_pair("/setwebhook", json::kind::object));
                if(opt.has_value() == false)
                {
                    return std::nullopt;
                }
                
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair("/setwebhook/url", json::kind::uint64)
                );

                if(map.size() != req_fields)
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
                <json::kind::string>(std::forward<T>(map), RFP(url, Utils::forward_like<T>(url)));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(certificate, Utils::forward_like<T>(certificate)));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(ip_address, ip_address));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), RFP(max_connections, max_connections));

                MainParser::field_from_map
                <json::kind::array>(std::forward<T>(map), RFP(allowed_updates, Utils::forward_like<T>(allowed_updates)));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), RFP(drop_pending_updates, drop_pending_updates));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(secret_token, Utils::forward_like<T>(secret_token)));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self) 
            {
                return SetWebHook::fields_to_value
                (
                    forward_like<Self>(self.url),
                    forward_like<Self>(self.certificate),
                    forward_like<Self>(self.ip_address),
                    self.max_connections,
                    forward_like<Self>(self.allowed_updates),
                    self.drop_pending_updates,
                    forward_like<Self>(self.secret_token)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string url,
                optstr certificate = {},
                optstr ip_address  = {},
                optint max_connections = {},
                std::optional<json::array> allowed_updates = {},
                optbool drop_pending_updates = {},
                optstr  secret_token         = {}
            )
            {
                json::object ob (MainParser::get_storage_ptr());
                json::object ob1(MainParser::get_storage_ptr());
                json::object ob2(MainParser::get_storage_ptr());

                ob = MainParser::parse_ObjPairs_as_obj
                   (
                        PAIR(url, std::move(url))
                   );


                ob1 = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(certificate, std::move(certificate)),
                        MAKE_OP(ip_address, std::move(ip_address)),
                        MAKE_OP(max_connections, max_connections),
                        MAKE_OP(allowed_updates, std::move(allowed_updates)),
                        MAKE_OP(drop_pending_updates, drop_pending_updates),
                        MAKE_OP(secret_token, std::move(secret_token))
                    ); 

                Pars::MainParser::container_move(std::move(ob1), ob);

                ob2["setwebhook"] = { std::move(ob) };
                return ob2;
            }
        };
        
    }//namespace TG

}//namespace Pars
