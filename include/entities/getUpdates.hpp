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
            std::optional<std::vector<json::string>> allowed_updates{};

            public:

            getUpdates
            (
                optint offset = {},
                optint limit  = {},
                optint timeout= {},
                std::optional<std::vector<json::string>> allowed_updates = {}
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
            json::string
            fields_to_url()
            {
                json::string off{FIELD_NAME(offset)"="};
                off += MainParser::parse_opt_as_string(offset);

                json::string lim{FIELD_NAME(limit)"="};
                lim += MainParser::parse_opt_as_string(limit);

                json::string time{FIELD_NAME(timeout)"="};
                time += MainParser::parse_opt_as_string(timeout);

                json::string updates;
                if (allowed_updates.has_value())
                {
                    for(auto&& i : allowed_updates.value())
                    {
                        updates += FIELD_NAME(allowed_updates)"=";
                        updates += i;
                        updates += "&";
                    }
                }
                updates.pop_back();

                json::string req{"/getupdates?"};
                req += std::move(off);  req += "&";
                req += std::move(lim);  req += "&"; 
                req += std::move(time); req += "&";
                req += std::move(updates);

                return req;
            }


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

                json::array arr;

                MainParser::field_from_map
                <json::kind::array>(map, std::make_pair(FIELD_NAME(allowed_updates), std::ref(arr)));

                MainParser::parse_jsonArray_to_container(allowed_updates.value(), arr);
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
