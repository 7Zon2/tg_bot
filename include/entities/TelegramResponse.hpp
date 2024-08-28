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
            optint error_code;
            optstr description;
            optobj result; 

            public:

            TelegramResponse(){}

            TelegramResponse
            (
                bool ok,
                optint error_code,
                optstrw description,
                optobj result
            )
            :
                ok(ok),
                error_code(error_code),
                description(description),
                result(result)
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

                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair("/ok", json::kind::bool_)
                );

                if(map.size() != sz)
                    return std::nullopt;
                else
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
                    std::make_pair("/error_code", json::kind::int64),
                    std::make_pair("/description", json::kind::string),
                    std::make_pair("/result", json::kind::object)
                );
                return map;
            }


            template<is_fields_map T>
            void fields_from_map
            (T&& map)
            {
                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), std::make_pair("ok", std::ref(ok)));

                MainParser::field_from_map
                <json::kind::uint64>(std::forward<T>(map), std::make_pair("error_code", std::ref(error_code)));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), std::make_pair("description", std::ref(description)));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), std::make_pair("result", std::ref(result)));
            }

            
            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::TelegramResponse
                (
                    ok,
                    error_code,
                    description,
                    result
                );
            }
        };

    }//namespace TG

}//namespace Pars
