#pragma once 
#include "TelegramEntities.hpp"
#include "User.hpp"
#include "Chat.hpp"

namespace Pars
{
    namespace TG
    {
        struct message : TelegramEntities<message>
        {
            using TelegramEntities::operator =; 

            using optuser = std::optional<User>;
            using optchat = std::optional<Chat>;

            public:

            uint64_t message_id;
            uint64_t date;
            Chat chat;

            optint  message_thread_id;
            optuser from;
            optchat sender_chat;
            optint  sender_boost_count;
            optuser sender_business_bot;
            optstr  business_connection_id;
            std::optional<std::shared_ptr<MessageOrigin>> forward_origin;
            optbool is_topic_message;
            optbool is_automatic_forward;
            std::optional<std::shared_ptr<message>> reply_to_message;
            ///fields
            ///
            ///....
            optuser via_bot;
            optuint edit_date;
            optbool has_protected_content;
            optbool is_from_offline;
            optstr media_group_id;
            optstr author_signature;
            optstr text;
             

            static constexpr size_t req_fields = 3;
            static constexpr size_t opt_fields = 17;

            public:

            message(){}

            message
            (
                uint64_t message_id,
                uint64_t date,
                Chat chat,
                optint  message_thread_id = {},
                optuser from = {},
                optchat sender_chat = {},
                optint  sender_boost_count = {},
                optuser sender_business_bot = {},
                optstr  business_connection_id = {},
                std::optional<std::shared_ptr<MessageOrigin>> forward_origin = {},
                optbool is_topic_message = {},
                optbool is_automatic_forward = {},
                std::optional<std::shared_ptr<message>> reply_to_message = {},
                optuser via_bot = {},
                optuint edit_date = {},
                optbool has_protected_content = {},
                optbool is_from_offline = {},
                optstr media_group_id = {},
                optstr author_signature = {},
                optstr text = {}
            )
            :
                message_id(message_id),
                date(date),
                chat(std::move(chat)),
                message_thread_id(message_thread_id),
                from(std::move(from)),
                sender_chat(std::move(sender_chat)),
                sender_boost_count(sender_boost_count),
                sender_business_bot(std::move(sender_business_bot)),
                business_connection_id(std::move(business_connection_id)),
                forward_origin(std::move(forward_origin)),
                is_topic_message(is_topic_message),
                is_automatic_forward(is_automatic_forward),
                reply_to_message(std::move(reply_to_message)),
                via_bot(std::move(via_bot)),
                edit_date(edit_date),
                has_protected_content(has_protected_content),
                is_from_offline(is_from_offline),
                media_group_id(std::move(media_group_id)),
                author_signature(std::move(author_signature)),
                text(std::move(text))
                {

                }

            public:

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                auto message_map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(message, message_id), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, date), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, user), json::kind::object)
                );

                if (message_map.size() != req_fields)
                {
                    return std::nullopt;
                }

                return message_map; 
            }


            [[nodiscard]]
            static 
            fields_map
            optional_fields(json::value val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(message, message_thread_id), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, from), json::kind::object),
                    std::make_pair(JS_POINTER(message, sender_chat), json::kind::object),
                    std::make_pair(JS_POINTER(message, sender_boost_count), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, sender_business_bot), json::kind::object),
                    std::make_pair(JS_POINTER(message, forward_origin), json::kind::object),
                    std::make_pair(JS_POINTER(message, is_topic_message), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, is_automatic_forward), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, reply_to_message), json::kind::object),
                    std::make_pair(JS_POINTER(message, via_bot), json::kind::object),
                    std::make_pair(JS_POINTER(message, edit_date), json::kind::uint64),
                    std::make_pair(JS_POINTER(message, has_protected_content), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, is_from_offline), json::kind::bool_),
                    std::make_pair(JS_POINTER(message, media_group_id), json::kind::string),
                    std::make_pair(JS_POINTER(message, author_signature), json::kind::string),
                    std::make_pair(JS_POINTER(message, text), json::kind::string)
                );
            };



            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(message_id));

               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(date));
     
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(chat));
                   
               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(message_thread_id));
                            
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(from));
                           
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(sender_chat));
                              
               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(sender_boost_count));
                                          
               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(sender_boost_count));
                                          
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(sender_business_bot));
                                          
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(forward_origin));
                                                                               
               MainParser::field_from_map
               <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(is_topic_message));
                                                                          
               MainParser::field_from_map
               <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(is_automatic_forward));
                                                                                         
               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(reply_to_message));

               MainParser::field_from_map
               <json::kind::object>(std::forward<T>(map), MAKE_PAIR(via_bot));

               MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(edit_date));

               MainParser::field_from_map
               <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(is_from_offline));

               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(media_group_id));
               
               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(author_signature));

               MainParser::field_from_map
               <json::kind::string>(std::forward<T>(map), MAKE_PAIR(text));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return TelegramRequestes::Message
                (
                    self.message_id,
                    self.date,
                    forward_like<Self>(self.chat),
                    self.message_thread_id,
                    forward_like<Self>(self.from),
                    forward_like<Self>(self.sender_chat),
                    self.sender_boost_count,
                    forward_like<Self>(self.sender_business_bot),
                    forward_like<Self>(self.business_connection_id),
                    forward_like<Self>(self.forward_origin),
                    self.is_topic_message,
                    self.is_automatic_forward,
                    forward_like<Self>(self.reply_to_message),
                    forward_like<Self>(self.via_bot),
                    self.edit_date,
                    self.has_protected_content,
                    self.is_from_offline,
                    forward_like<Self>(self.media_group_id),
                    forward_like<Self>(self.author_signature),
                    forward_like<Self>(self.text)
                );
            }
        };

    }//namespace TG   

}//namespace Pars