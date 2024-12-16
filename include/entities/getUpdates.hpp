#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct getUpdates : TelegramEntities<getUpdates>
        {
            using TelegramEntities::operator=;

            optint offset{};
            optint limit{};
            optint timeout{};
            std::optional<std::vector<json::string>> allowed_updates{};

            static  constexpr size_t req_fields = 0;
            static  constexpr size_t opt_fields = 4;
            static  const inline json::string entity_name{"getUpdates"}; 

            public:

            [[nodiscard]]
            json::string
            get_entity_name() override
            {
                return entity_name;
            }

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
             allowed_updates(std::move(allowed_updates))
            {

            }

            public:

            [[nodiscard]]
            json::string
            fields_to_url() 
            {
                json::string off{FIELD_EQUAL(offset)};
                off += MainParser::parse_opt_as_string(offset);

                json::string lim{FIELD_EQUAL(limit)};
                lim += MainParser::parse_opt_as_string(limit);

                json::string time{FIELD_EQUAL(timeout)};
                time += MainParser::parse_opt_as_string(timeout);

                json::string updates;
                if (allowed_updates.has_value())
                {
                    for(auto&& i : allowed_updates.value())
                    {
                        updates += FIELD_EQUAL(allowed_updates);
                        updates += i;
                        updates += "&";
                    }
                    updates.pop_back();
                }

                json::string req{"/getupdates?"};
                req += std::move(off);  req += "&";
                req += std::move(lim);  req += "&"; 
                req += std::move(time); req += "&";
                req += std::move(updates);

                return req;
            }


            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(json::value val)
            {
                return std::unordered_map<json::string, json::value>{};
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(getupdates, offset), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, limit), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, timeout), json::kind::int64),
                    std::make_pair(JS_POINTER(getupdates, allowed_updates), json::kind::array)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map
            (T&& map)
            {
                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(offset, offset));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(limit, limit));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(timeout, timeout));

                json::array arr;

                MainParser::field_from_map
                <json::kind::array>(std::forward<T>(map), std::make_pair(FIELD_NAME(allowed_updates), std::ref(arr)));

                allowed_updates = std::vector<json::string>{};
                MainParser::parse_jsonArray_to_container(allowed_updates.value(), arr); 
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return getUpdates::fields_to_value
                (
                    self.offset,
                    self.limit,
                    self.timeout,
                    forward_like<Self>(self.allowed_updates)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                optint offset = {},
                optint limit  = {},
                optint timeout= {},
                std::optional<std::vector<json::string>> allowed_updates = {}
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob = MainParser::parse_OptPairs_as_obj
                (
                    MAKE_OP(offset, offset),
                    MAKE_OP(limit, limit),
                    MAKE_OP(timeout, timeout)        
                );

                json::array arr;
                if(allowed_updates.has_value())
                {
                    for(auto&& i : allowed_updates.value())
                    {
                        arr.push_back(std::move(i));
                    }
                }

                ob.insert_or_assign(FIELD_NAME(allowed_updates), std::move(arr));

                json::object ob2(MainParser::get_storage_ptr());
                ob2["getupdates"] = std::move(ob);
                return ob2;
            }

        };
    }
}
