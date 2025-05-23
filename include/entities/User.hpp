#pragma once
#include "TelegramEntities.hpp"

namespace Pars
{
    namespace TG
    {
        struct User : TelegramEntities<User>
        {
            public:

            using TelegramEntities::operator = ;

            static constexpr const size_t req_fields  = 3;
            static constexpr const size_t opt_fields  = 9;
            static const inline json::string entity_name {"user"};

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            uint64_t id ={};
            bool is_bot = {};
            json::string first_name = {};
            optstr  last_name     = {};
            optstr  username      = {};
            optstr  language_code = {};
            optbool is_premium    = {};
            optbool added_to_attachment_menu     = {};
            optbool can_join_groups              = {};
            optbool can_read_all_group_messages  = {};
            optbool supports_inline_queries      = {};
            optbool can_connect_to_business      = {};

            public:

            User(){}
            ~User(){}

            template<is_all_json_entities T>
            User(T&& val)
            {
                create(std::forward<T>(val));
            }

            User
            (
                uint64_t id,
                bool is_bot,
                json::string first_name,
                optstr last_name     = {},
                optstr username      = {},
                optstr language_code = {},
                optbool is_premium    = {},
                optbool added_to_attachment_menu        = {},
                optbool can_join_groups                 = {},
                optbool can_read_all_group_messages     = {},
                optbool supports_inline_queries         = {},
                optbool can_connect_to_business         = {}
            )
            :
                id(id),
                is_bot(is_bot),
                first_name(std::move(first_name)),
                last_name(std::move(last_name)),
                username (std::move(username)),
                language_code(std::move(language_code)),
                is_premium(is_premium),
                added_to_attachment_menu(added_to_attachment_menu),
                can_join_groups(can_join_groups),
                can_read_all_group_messages(can_read_all_group_messages),
                supports_inline_queries(supports_inline_queries),
                can_connect_to_business(can_connect_to_business)
            {

            } 


            public:


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map 
            requested_fields(T&& val) 
            {
               
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JSP(id), json::kind::uint64),
                    std::make_pair(JSP(is_bot), json::kind::bool_),
                    std::make_pair(JSP(first_name), json::kind::string)
                );

                if(map.size() != req_fields)
                {
                    return std::nullopt;
                }

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
                    std::make_pair(JSP(last_name), json::kind::string),
                    std::make_pair(JSP(username),  json::kind::string),
                    std::make_pair(JSP(language_code), json::kind::string),
                    std::make_pair(JSP(is_premium), json::kind::bool_),
                    std::make_pair(JSP(added_to_attachment_menu), json::kind::bool_),
                    std::make_pair(JSP(can_join_groups), json::kind::bool_),
                    std::make_pair(JSP(can_read_all_group_messages), json::kind::bool_),
                    std::make_pair(JSP(supports_inline_queries), json::kind::bool_),
                    std::make_pair(JSP(can_connect_to_business), json::kind::bool_)
                );
            }


            template<is_fields_map T>
            void fields_from_map
            (T && map)
            {
                print("\nuser map:\n");
                MainParser::print_map(map);
                print("\n\n");

                MainParser::field_from_map
                <json::kind::uint64>( std::forward<T>(map),  std::make_pair("id", std::ref(id)));

                MainParser::field_from_map
                <json::kind::bool_>( std::forward<T>(map),  std::make_pair("is_bot", std::ref(is_bot)));

                MainParser::field_from_map
                <json::kind::string>(  std::forward<T>(map),  std::make_pair("first_name", std::ref(first_name)));

                MainParser::field_from_map
                <json::kind::string>(  std::forward<T>(map),  std::make_pair("last_name", std::ref(last_name)));

                MainParser::field_from_map
                <json::kind::string>(  std::forward<T>(map),  std::make_pair("username",  std::ref(username)));

                MainParser::field_from_map
                <json::kind::string>(  std::forward<T>(map),  std::make_pair("language_code", std::ref(language_code)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("is_premium", std::ref(is_premium)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("added_to_attachment_menu", std::ref(added_to_attachment_menu)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("can_join_groups", std::ref(can_join_groups)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("can_read_all_group_messages", std::ref(can_read_all_group_messages)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("supports_inline_queries", std::ref(supports_inline_queries)));

                MainParser::field_from_map
                <json::kind::bool_>(   std::forward<T>(map),  std::make_pair("can_connect_to_business", std::ref(can_connect_to_business)));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return User::fields_to_value
                (
                    self.id,
                    self.is_bot,
                    Utils::forward_like<Self>(self.first_name),
                    Utils::forward_like<Self>(self.last_name),
                    Utils::forward_like<Self>(self.username),
                    Utils::forward_like<Self>(self.language_code),
                    self.is_premium,
                    self.added_to_attachment_menu,
                    self.can_join_groups,
                    self.can_read_all_group_messages,
                    self.supports_inline_queries,
                    self.can_connect_to_business
                );
            }

            [[nodiscard]]
            static json::value
            fields_to_value
            (
                uint64_t id,
                bool is_bot,
                json::string first_name,
                optstr last_name                    = {},
                optstr username                     = {},
                optstr language_code                = {},
                optbool is_premium                  = {},
                optbool added_to_attachment_menu    = {},
                optbool can_join_groups             = {},
                optbool can_read_all_group_messages = {},
                optbool supports_inline_queries     = {},
                optbool can_connect_to_business     = {}
            )
            {
                json::object ob   {MainParser::get_storage_ptr()};
                json::object ob_1 {MainParser::get_storage_ptr()};
                json::object ob_2 {MainParser::get_storage_ptr()};

                ob_1 =  MainParser::parse_ObjPairs_as_obj
                        (
                            PAIR(id, id),
                            PAIR(is_bot, is_bot),
                            PAIR(first_name, std::move(first_name))
                        );

                ob_2 =  MainParser::parse_OptPairs_as_obj
                        (
                            MAKE_OP(last_name, std::move(last_name)),
                            MAKE_OP(username, std::move(username)),
                            MAKE_OP(language_code, std::move(language_code)),
                            MAKE_OP(is_premium, is_premium),
                            MAKE_OP(added_to_attachment_menu, added_to_attachment_menu),
                            MAKE_OP(can_join_groups, can_join_groups),
                            MAKE_OP(can_read_all_group_messages, can_read_all_group_messages),
                            MAKE_OP(supports_inline_queries, supports_inline_queries),
                            MAKE_OP(can_connect_to_business, can_connect_to_business)    
                        );

                MainParser::container_move(std::move(ob_2), ob_1);

                ob["user"] = { std::move(ob_1) };
                return ob;
            }
        };
    }//namespace TG
}//namespace Pars

