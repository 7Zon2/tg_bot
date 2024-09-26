#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct Chat : TelegramEntities<Chat>
        {
            using TelegramEntities<Chat>::operator = ;

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

            Chat(){}

            Chat
            (
                const size_t id,
                json::string type,
                optstr title = {},
                optstr username = {},
                optstr first_name = {},
                optstr last_name  = {},
                optbool is_forum  = {}
            )
            :
                id(id),
                type(std::move(type)),
                title(std::move(title)),
                username(std::move(username)),
                first_name(std::move(first_name)),
                last_name(std::move(last_name)),
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


            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                auto map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(Chat, id), json::kind::uint64),
                    std::make_pair(JS_POINTER(Chat, type), json::kind::string)
                );            

                if (map.size() != req_fields)
                    return std::nullopt;
                else
                    return map;
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(Chat, title), json::kind::string),
                    std::make_pair(JS_POINTER(Chat, username), json::kind::string),
                    std::make_pair(JS_POINTER(Chat, first_name), json::kind::string),
                    std::make_pair(JS_POINTER(Chat, last_name), json::kind::string),
                    std::make_pair(JS_POINTER(Chat, is_forum),  json::kind::bool_)
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
                return Chat::fields_to_value
                (
                    self.id, 
                    Utils::forward_like<Self>(self.type),
                    Utils::forward_like<Self>(self.title),
                    Utils::forward_like<Self>(self.username),
                    Utils::forward_like<Self>(self.first_name),
                    Utils::forward_like<Self>(self.last_name),
                    self.is_forum
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                uint64_t id,
                json::string type,
                optstr title = {},
                optstr username = {},
                optstr first_name = {},
                optstr last_name  = {},
                optbool is_forum   = {}
            )
            {
                json::object ob(MainParser::get_storage_ptr());
                ob = MainParser::parse_ObjPairs_as_obj
                (
                    PAIR(id),
                    PAIR(std::move(type))
                );

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = MainParser::parse_OptPairs_as_obj
                (
                    MAKE_OP(std::move(title)),
                    MAKE_OP(std::move(username)),
                    MAKE_OP(std::move(first_name)),
                    MAKE_OP(std::move(last_name)),
                    MAKE_OP(is_forum)
                );

                MainParser::container_move(ob2, ob);

                json::object res(MainParser::get_storage_ptr());
                res[FIELD_TO_LOWER(chat)] = std::move(ob);
                return res;
            }
        };
    }
}