#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct Chat : TelegramEntities<Chat>
        {
            using TelegramEntities<Chat>::operator = ;

            static constexpr const  size_t req_fields = 2;
            static constexpr const  size_t opt_fields = 5;
            static inline const  json::string entity_name{"chat"};

            public:

            size_t id;
            json::string type;
            optstr title;
            optstr username;
            optstr first_name;
            optstr last_name;
            optbool is_forum;


            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            Chat(){}

            template<is_all_json_entities T>
            Chat(T&& val)
            {
                create(std::forward<T>(val));
            }

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


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                print("\nchat::\n");
                MainParser::pretty_print(std::cout, val);
                print("\n\n");
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JSP(id), json::kind::int64),
                    std::make_pair(JSP(type), json::kind::string)
                );            

                if (map.size() != req_fields)
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
                    std::make_pair(JSP(title), json::kind::string),
                    std::make_pair(JSP(username), json::kind::string),
                    std::make_pair(JSP(first_name), json::kind::string),
                    std::make_pair(JSP(last_name), json::kind::string),
                    std::make_pair(JSP(is_forum),  json::kind::bool_)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map
            (T&&  map)
            {   
                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), RFP(id, id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(type, type));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(title, title));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(username, username));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(first_name, first_name));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), RFP(last_name, last_name));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), RFP(is_forum, is_forum));
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
                    PAIR(id, id),
                    PAIR(type, std::move(type))
                );

                json::object ob2(MainParser::get_storage_ptr());
                ob2 = MainParser::parse_OptPairs_as_obj
                (
                    MAKE_OP(title, std::move(title)),
                    MAKE_OP(username, std::move(username)),
                    MAKE_OP(first_name, std::move(first_name)),
                    MAKE_OP(last_name, std::move(last_name)),
                    MAKE_OP(is_forum, is_forum)
                );

                MainParser::container_move(ob2, ob);

                json::object res(MainParser::get_storage_ptr());
                res[FIELD_TO_LOWER(chat)] = std::move(ob);
                return res;
            }
        };
    }
}
