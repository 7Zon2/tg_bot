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

            uint64_t id;
            bool is_bot;
            json::string first_name;
            optstr  last_name     = {};
            optstr  username      = {};
            optstr  language_code = {};
            optbool is_premium    = {};
            optbool added_to_attachment_menu     = {};
            optbool can_join_groups              = {};
            optbool can_read_all_group_messages  = {};
            optbool supports_inline_queries      = {};
            optbool can_connect_to_business      = {};

            static inline size_t req_fields  = 3;
            static inline size_t opt_fields  = 9; 

            public:

            User(){}

            User
            (
                uint64_t id,
                bool is_bot,
                json::string_view first_name,
                optstrw last_name     = {},
                optstrw username      = {},
                optstrw language_code = {},
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
                first_name(first_name),
                last_name(last_name),
                username (username),
                language_code(language_code),
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
                const size_t sz = 3;

                auto opt = MainParser::check_pointer_validation(std::forward<T>(val), std::make_pair("/user", json::kind::object));
                if(opt.has_value() == false)
                {
                    return std::nullopt;
                }
                

                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair("/user/id", json::kind::uint64),
                    std::make_pair("/user/is_bot", json::kind::bool_),
                    std::make_pair("/user/first_name", json::kind::string)
                );

                if(map.size() < sz)
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
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair("/user/last_name", json::kind::string),
                    std::make_pair("/user/username",  json::kind::string),
                    std::make_pair("/user/language_code", json::kind::string),
                    std::make_pair("/user/is_premium", json::kind::bool_),
                    std::make_pair("/user/added_to_attachment_menu", json::kind::bool_),
                    std::make_pair("/user/can_join_groups", json::kind::bool_),
                    std::make_pair("/user/can_read_all_group_messages", json::kind::bool_),
                    std::make_pair("/user/supports_inline_queries", json::kind::bool_),
                    std::make_pair("/user/can_connect_to_business", json::kind::bool_)
                );

                return map;
            }


            template<as_json_value T>
            void fields_from_map
            (T && map)
            {
                MainParser::field_from_map
                <json::kind::uint64>( std::forward<T>(map),  std::make_pair("id", std::ref(id)));

                MainParser::field_from_map
                <json::kind::bool_>( std::forward<T>(map),  std::make_pair("is_bot", std::ref(is_bot)));

                MainParser::field_from_map
                <json::kind::string>( std::forward<T>(map),  std::make_pair("first_name", std::ref(first_name)));

                MainParser::field_from_map
                <json::kind::string>( std::forward<T>(map),  std::make_pair("last_name", std::ref(last_name)));

                MainParser::field_from_map
                <json::kind::string>( std::forward<T>(map),  std::make_pair("username",  std::ref(username)));

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

            public:

            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::get_user_request
                (
                    id,
                    is_bot,
                    first_name,
                    last_name,
                    username,
                    language_code,
                    is_premium,
                    added_to_attachment_menu,
                    can_join_groups,
                    can_read_all_group_messages,
                    supports_inline_queries,
                    can_connect_to_business
                );
            }
        };
    }//namespace TG
}//namespace Pars

