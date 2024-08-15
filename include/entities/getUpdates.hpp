#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct getUpdates : TelegramEntities<getUpdates>
        {
            optint offset{};
            optint limit{};
            optint timeout{};
            std::optional<json::array> allowed_updates{};

            public:

            getUpdates(){}

            getUpdates
            (
                optint offset = {},
                optint limit  = {},
                optint timeout= {},
                std::optional<json::array> allowed_updates = {}
            )
            :
             offset(offset),
             limit(limit),
             timeout(timeout),
             allowed_updates(allowed_updates)
            {

            }

            public:

            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>>
            requested_fields(const json::value& val)
            {
                return std::unordered_map<json::string, json::value>{};
            }


            [[nodiscard]]
            static
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(JS_POINTER(getupdates, offset), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, limit), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, timeout), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, allowed_updates), json::kind::array)
                );
            }


            void 
            fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::int64>(map, MAKE_PAIR(offset));

                MainParser::field_from_map
                <json::kind::int64>(map, MAKE_PAIR(limit));

                MainParser::field_from_map
                <json::kind::int64>(map, MAKE_PAIR(timeout));

                MainParser::field_from_map
                <json::kind::array>(map, MAKE_PAIR(allowed_updates));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::getUpdates
                (
                    offset,
                    limit,
                    timeout,
                    allowed_updates
                );
            }
        };
    }
}
