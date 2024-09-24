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
            size_t date_;

            static constexpr size_t req_fields = 2;
            static constexpr size_t opt_fields = 0;

            public:

            MessageOrigin(){}

            MessageOrigin(json::string type, const size_t date)
                : type(std::move(type)), date_(date){}

            virtual ~MessageOrigin() = 0;

            public:


            template<as_json_value T>
            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(T&& val, json::string_view method)
            {
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

                if (req_fields!=map.size())
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
                <json::kind::uint64>(std::forward<T>(map), std::make_pair("date",std::ref(date_)));
            }
        };

        template<typename T>
        MessageOrigin<T>::~MessageOrigin(){}


        struct MessageOriginUser : MessageOrigin
        {

            using MessageOrigin::operator=;

            User sender_user;

            static constexpr size_t req_fields = 3;
            static constexpr size_t opt_fields = 0;

            public:

            MessageOriginUser(){}

            MessageOriginUser
            (
                json::string type,
                const size_t date,
                TG::User user
            )
            :
             MessageOrigin(std::move(type), date),
             sender_user(std::move(user))
             {

             }

             public:


            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(json::value val)
            {
                auto map = MessageOrigin::requested_fields(std::move(val), FIELD_NAME(messageoriginuser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(messageoriginuser, sender_user), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                MainParser::container_move(std::move(map2), map_);
                return std::move(map_);
            }


            static
            fields_map
            optional_fields(json::value val)
            {
                return MessageOrigin::optional_fields(std::move(val));
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::MessageOriginUser
                (
                    type,
                    date_,
                    std::move(sender_user)
                );
            }
        };


        struct MessageOriginHiddenUser : MessageOrigin
        {
            using MessageOrigin::operator=;

            json::string sender_user_name;

            static inline size_t req_fields = 3;
            static inline size_t opt_fields = 0;

            public:

            MessageOriginHiddenUser(){}

            MessageOriginHiddenUser
            (
                json::string type,
                const size_t date,
                json::string sender_user_name
            )
            :
            MessageOrigin(std::move(type), date),
            sender_user_name(std::move(sender_user_name))
            {

            }

            public:

            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(json::value val)
            {
                auto map = MessageOrigin::requested_fields(std::move(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(messageoriginhiddenuser, sender_user_name), json::kind::string)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                auto & map_ = map.value();
                MainParser::container_move(std::move(map2), map_);

                return std::move(map_);
            }


            static
            fields_map
            optional_fields(json::value val)
            {
                return MessageOrigin::optional_fields(std::move(val));
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::MessageOriginHiddenUser
                (
                    type,
                    date_,
                    sender_user_name
                );
            }
        };


        struct MessageOriginChat : MessageOrigin
        {
            using MessageOrigin::operator=;

            TG::chat sender_chat;
            optstr  author_signature = {};

            static inline size_t req_fields = 3;
            static inline size_t opt_fields = 1;

            public:

            MessageOriginChat(){}

            MessageOriginChat
            (
                json::string type,
                const size_t date,
                TG::chat sender_chat,
                optstr author_signature = {}
            )
            :
            MessageOrigin(std::move(type), date),
            sender_chat(std::move(sender_chat)),
            author_signature(std::move(author_signature))
            {

            }

            public:


            [[nodiscard]]
            static 
            opt_fields_map
            requested_fields(json::value val)
            {
                auto map = MessageOrigin::requested_fields(std::move(val), FIELD_NAME(MessageOriginHiddenUser));
                if(map.has_value() == false)
                {
                    return map;
                }

                auto map2 = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(MessageOriginChat, sender_chat), json::kind::object)
                );

                if(map2.empty())
                {
                    return std::nullopt;
                }

                MainParser::container_move(std::move(map2), map_);
                return std::move(map_);
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
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


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::MessageOriginChat
                (
                    type,
                    date_,
                    forward_like<Self>(sender_chat),
                    forward_like<Self>(author_signature)
                );
            }
        };
    }
}