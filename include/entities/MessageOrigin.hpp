#pragma once
#include "TelegramEntities.hpp"
#include"Chat.hpp"
#include "User.hpp"


namespace Pars
{
    namespace TG
    {

        template<typename T>
        concept is_user = std::is_same_v<std::remove_reference_t<T>, User>;

        template<typename T>
        concept is_chat = std::is_same_v<std::remove_reference_t<T>, chat>;


        struct MessageOrigin : TelegramEntities<MessageOrigin>
        {
            json::string type;
            size_t date;

            public:

            MessageOrigin(){}

            MessageOrigin(json::string_view type, const size_t date)
                : type(type), date(date){}

            virtual ~MessageOrigin() = 0;

            public:


            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>>
            requested_fields(const json::value& val, json::string_view method)
            {
                const size_t sz = 2;

                json::string meth{"/"};
                meth+=method;
                meth+="/";

                json::string type_{method};
                type_+="type";

                json::string date_{method};
                date_+="date";

                auto map =  MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(type_, json::kind::string),
                    std::make_pair(date_, json::kind::uint64)
                );
            }


            static
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return {};
            }


            virtual void 
            fields_from_map
            (const std::unordered_map<json::string, json::value>& map)
            {
                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(type));

                MainParser::field_from_map
                <json::kind::uint64>(map, MAKE_PAIR(date));
            }
        };

        MessageOrigin::~MessageOrigin(){}


        struct MessageOriginUser : MessageOrigin
        {
            User sender_user;

            public:

            MessageOriginUser(){}

            template<is_user T>
            MessageOriginUser
            (
                json::string_view type,
                const size_t date,
                T&& user
            )
            :
             MessageOrigin(type, date),
             sender_user(std:forward<T>(user))
             {

             }

             public:

            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>>
            requested_fields(const json::value& val)
            {
                auto map = MessageOrigin::requested_fields(val, FIELD_NAME(messageoriginuser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(JS_POINTER(messageoriginuser, sender_user), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.emplace(b,e);

                return std::move(map_);
            }


            static
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return MessageOrigin::optional_fields(val);
            }


            void
            fields_from_map
            (const std::unordered_map<json::string, json::value> & map)
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::object>(map, MAKE_PAIR(sender_user));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::MessageOriginUser
                (
                    type,
                    date,
                    sender_user
                );
            }
        };


        struct MessageOriginHiddenUser : MessageOrigin
        {
            json::string sender_user_name;

            public:

            MessageOriginHiddenUser(){}

            MessageOriginHiddenUser
            (
                json::string_view type,
                const size_t date,
                json::string_view sender_user_name
            )
            :
            MessageOrigin(type, date),
            sender_user_name(sender_user_name)
            {

            }

            public:

            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>>
            requested_fields(const json::value& val)
            {
                auto map = MessageOrigin::requested_fields(val, FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(JS_POINTER(messageoriginhiddenuser, sender_user_name), json::kind::string)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.emplace(b,e);

                return std::move(map_);
            }


            static
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return MessageOrigin::optional_fields(val);
            }


            void
            fields_from_map
            (const std::unordered_map<json::string, json::value> & map)
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(sender_user_name));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::MessageOriginHiddenUser
                (
                    type,
                    date,
                    sender_user_name
                );
            }
        };


        struct MessageOriginChat : MessageOrigin
        {
            TG::chat sender_chat;
            optstrw  author_signature = {};

            public:

            MessageOriginChat(){}

            template<TG::is_chat T>
            MessageOriginChat
            (
                json::string_view type,
                const size_t date,
                T&& sender_chat,
                optstrw author_signature = {}
            )
            :
            MessageOrigin(type, date),
            sender_chat(std::forward<T>(sender_chat)),
            author_signature(author_signature)
            {

            }

            public:


            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string, json::value>>
            requested_fields(const json::value& val)
            {
                auto map = MessageOrigin::requested_fields(val, FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(JS_POINTER(MessageOriginChat, sender_chat), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.emplace(b,e);

                return std::move(map_);
            }

            [[nodiscard]]
            static
            std::unordered_map<json::string, json::value>
            optional_fields(const json::value& val)
            {
                return MainParser::mapped_pointers_validation
                (
                    val,
                    std::make_pair(JS_POINTER(messageoriginchat, author_signature), json::kind::string)
                );
            }


            void
            fields_from_map
            (const std::unordered_map<json::string, json::value> & map)
            {
                MessageOrigin::fields_from_map(map);

                MainParser::field_from_map
                <json::kind::object>(map, MAKE_PAIR(sender_chat));

                MainParser::field_from_map
                <json::kind::string>(map, MAKE_PAIR(author_signature));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::MessageOriginChat
                (
                    type,
                    date,
                    sender_chat,
                    author_signature
                );
            }
        };
    }
}