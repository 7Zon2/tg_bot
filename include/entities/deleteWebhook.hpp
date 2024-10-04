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

            using TelegramEntities::operator =;

            bool drop_pending_updates;

            static constexpr size_t req_fields = 1;
            static constexpr size_t opt_fields = 0;
            static const inline json::string entity_name {"deletewebhook"};

            public:

            deletewebhook(){}

            deletewebhook(bool drop_pending_updates)
            :drop_pending_updates(drop_pending_updates){}


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map 
            requested_fields(T&& val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(deletewebhook, drop_pending_updates), json::kind::bool_)
                );

                if (map.size() != req_fields)
                {
                    return {};
                }
                return map;
            }


            template<as_json_value T>
            [[nodiscard]]
            static 
            fields_map
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return deletewebhook::fields_to_value
                (
                    self.drop_pending_updates
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                bool drop_pending_updates
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob["drop_pending_updates"] = drop_pending_updates;
                return ob;
            }
            
        };
    }
}

