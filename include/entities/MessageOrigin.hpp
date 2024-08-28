#pragma once
#include "TelegramEntities.hpp"
#include "concept_entities.hpp"

namespace Pars
{
    namespace TG
    {
        struct MessageOrigin : TelegramEntities<MessageOrigin>
        {
            using TelegramEntities::operator=;

            json::string type;
            size_t date;

            public:

            MessageOrigin(){}

            MessageOrigin(json::string_view type, const size_t date)
                : type(type), date(date){}

            virtual ~MessageOrigin() = 0;

            public:


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val, json::string_view method)
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
                    std::forward<T>(val),
                    std::make_pair(type_, json::kind::string),
                    std::make_pair(date_, json::kind::uint64)
                );

                if (sz!=map.size())
                {
                    return std::nullopt;
                }

                return map;
            }


            template<as_json_value T>
            static
            fields_map
            optional_fields(T&& val)
            {
                return {};
            }


            template<is_fields_map T>
            void 
            fields_from_map
            (T&& map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(type));

                MainParser::field_from_map
                <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(date));
            }
        };

        MessageOrigin::~MessageOrigin(){}


        struct MessageOriginUser : MessageOrigin
        {

            using MessageOrigin::operator=;

            User sender_user;

            public:

            MessageOriginUser(){}

            template<TG::is_user T>
            MessageOriginUser
            (
                json::string_view type,
                const size_t date,
                T&& user
            )
            :
             MessageOrigin(type, date),
             sender_user(std::forward<T>(user))
             {

             }

             public:


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(messageoriginuser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginuser, sender_user), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.insert(b,e);

                return std::move(map_);
            }


            template<as_json_value T>
            static
            fields_map
            optional_fields(T&& val)
            {
                return MessageOrigin::optional_fields(std::forward<T>(val));
            }


            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
                MessageOrigin::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(sender_user));
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
            using MessageOrigin::operator=;

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

            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(messageoriginhiddenuser, sender_user_name), json::kind::string)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.insert(b,e);

                return std::move(map_);
            }


            template<as_json_value T>
            static
            fields_map
            optional_fields(T&& val)
            {
                return MessageOrigin::optional_fields(std::forward<T>(val));
            }


            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
                MessageOrigin::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(sender_user_name));
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
            using MessageOrigin::operator=;

            TG::chat sender_chat;
            optstr  author_signature = {};

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


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val)
            {
                auto map = MessageOrigin::requested_fields(std::forward<T>(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(MessageOriginChat, sender_chat), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                auto b = std::make_move_iterator(map2.begin());
                auto e = std::make_move_iterator(map2.end());
                map_.insert(b,e);

                return std::move(map_);
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
                    std::make_pair(JS_POINTER(messageoriginchat, author_signature), json::kind::string)
                );
            }


            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
                MessageOrigin::fields_from_map(std::forward<T>(map));

                MainParser::field_from_map
                <json::kind::object>(std::forward<T>(map), MAKE_PAIR(sender_chat));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(author_signature));
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