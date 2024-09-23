#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct chat : TelegramEntities<chat>
        {
            using TelegramEntities::operator = ;

            size_t id;
            json::string type;
            optstr title;
            optstr username;
            optstr first_name;
            optstr last_name;
            optbool is_forum;

            static inline size_t req_fields = 2;
            static inline size_t opt_fields = 5;

            public:

            chat(){}

            chat
            (
                const size_t id,
                json::string_view type,
                optstrw title = {},
                optstrw username = {},
                optstrw first_name = {},
                optstrw last_name  = {},
                optbool is_forum  = {}
            )
            :
                id(id),
                type(type),
                title(title),
                username(username),
                first_name(first_name),
                last_name(last_name),
                is_forum(is_forum)    
            {
               
            }

            public:

            [[nodiscard]]
            json::string
            fields_to_url() 
            {
                json::string id_{FIELD_EQUAL(id)};
                id_ += std::to_string(id);

                json::string type_{FIELD_EQUAL(type)};
                type_+=type;

                json::string title_{FIELD_EQUAL(title)};
                title_ += MainParser::parse_opt_as_string(title);

                json::string username_{FIELD_EQUAL(username_)};
                username_ = MainParser::parse_opt_as_string(username);

                json::string first_name_{FIELD_EQUAL(first_name)};
                first_name_ += MainParser::parse_opt_as_string(first_name);

                json::string last_name_{FIELD_EQUAL(last_name)};
                last_name_ += MainParser::parse_opt_as_string(last_name);

                json::string is_forum_{FIELD_EQUAL(is_forum)};
                is_forum_ += MainParser::parse_opt_as_string(is_forum);

                json::string req{"/chat?"};
                req += MainParser::concat_string
                (   
                    '&',
                     std::move(id_),
                     std::move(type_),
                     std::move(title_),
                     std::move(username_),
                     std::move(first_name_),
                     std::move(last_name_),
                     std::move(is_forum_)
                );

                return req;
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                const size_t sz = 2;
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(chat, id), json::kind::uint64),
                    std::make_pair(JS_POINTER(chat, type), json::kind::string)
                );            

                if (map.size() != sz)
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
                return MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(chat, title), json::kind::string),
                    std::make_pair(JS_POINTER(chat, username), json::kind::string),
                    std::make_pair(JS_POINTER(chat, first_name), json::kind::string),
                    std::make_pair(JS_POINTER(chat, last_name), json::kind::string),
                    std::make_pair(JS_POINTER(chat, is_forum),  json::kind::bool_)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map
            (T&&  map)
            {   
                MainParser::field_from_map
                <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(type));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(title));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(username));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(first_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(last_name));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(is_forum));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::chat
                (
                    id,
                    type,
                    title,
                    username,
                    first_name,
                    last_name,
                    is_forum
                );
            }
        };
    }
}