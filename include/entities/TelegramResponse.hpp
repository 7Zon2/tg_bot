#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct TelegramResponse : TelegramEntities<TelegramResponse>
        {
            public:

            bool ok;
            int error_code;
            json::string description; 

            public:

            TelegramResponse(){}

            TelegramResponse
            (
                bool ok,
                int error_code,
                json::string_view description
            )
            :
                ok(ok),
                error_code(error_code),
                description(description)
                {

                }

            public:

            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>> 
            requested_fields(const json::value& val)
            {
                const size_t sz = 3;

                auto map = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair("/ok", json::kind::bool_),
                    std::make_pair("/error_code", json::kind::int64),
                    std::make_pair("/description", json::kind::string)
                );

                if(map.size() != sz)
                    return std::nullopt;
                else
                    return map;                
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
                <json::kind::bool_>(map, std::make_pair("ok", std::ref(ok)));

                MainParser::field_from_map
                <json::kind::uint64>(map, std::make_pair("error_code", std::ref(error_code)));

                MainParser::field_from_map
                <json::kind::string>(map, std::make_pair("description", std::ref(description)));
            }

            
            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::TelegramResponse
                (
                    ok,
                    error_code,
                    description
                );
            }
        };

    }//namespace TG

}//namespace Pars
