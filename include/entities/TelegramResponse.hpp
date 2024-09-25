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
            optarray result; 

            static inline size_t req_fields = 1;
            static inline size_t opt_fields = 3;

            public:

            TelegramResponse(){}

            TelegramResponse
            (
                bool ok,
                optint error_code,
                optstr description,
                optarray result
            )
            :
                ok(ok),
                error_code(error_code),
                description(std::move(description)),
                result(std::move(result))
                {

                }

            public:

            [[nodiscard]]
            static 
            opt_fields_map 
            requested_fields(json::value val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair("/ok", json::kind::bool_)
                );

                if(map.size() != req_fields)
                    return std::nullopt;
                else
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
                    std::make_pair("/error_code", json::kind::int64),
                    std::make_pair("/description", json::kind::string),
                    std::make_pair("/result", json::kind::array)
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
                <json::kind::array>(std::forward<T>(map), std::make_pair("result", std::ref(result)));
            }

            
            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramResponse::fields_to_value
                (
                    self.ok,
                    self.error_code,
                    forward_like<Self>(self.description),
                    forward_like<Self>(self.result)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                bool ok,
                optint error_code,
                optstr description,
                optarray result
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob = parse_ObjPairs_as_obj(PAIR(ok));

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = parse_OptPairs_as_obj
                (
                    MAKE_OP(error_code),
                    MAKE_OP(std::move(description))
                );

                Pars::MainParser::container_move(std::move(ob2), ob);

                if (result.has_value())
                {
                    json::value val_arr{std::move(result.value())};
                    ob.emplace(FIELD_NAME(result), std::move(val_arr));
                }
                return ob;
            }
            
        };

    }//namespace TG

}//namespace Pars
