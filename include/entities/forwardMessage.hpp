#pragma once
#include "TelegramEntities.hpp"


namespace Pars
{
    namespace TG
    {
        struct forwardMessage : TelegramEntities<forwardMessage>
        {
            json::string chat_id;
            json::string from_chat_id;
            int message_id;
            optint message_thread_id;
            optbool disable_notification;
            optbool protect;

            static constexpr size_t req_fields = 3;
            static constexpr size_t opt_fields = 3;

            public:

            forwardMessage(){}

            forwardMessage
            (
                json::string chat_id,
                json::string from_chat_id,
                int message_id,
                optint message_thread_id,
                optbool disable_notification,
                optbool protect
            )
            :
             chat_id(std::move(chat_id)),
             from_chat_id(std::move(from_chat_id)),
             message_id(message_id),
             message_thread_id(message_thread_id),
             disable_notification(disable_notification),
             protect(protect)
             {
                
             }


             public:

            [[nodiscard]]
            json::string
            fields_to_url() 
            {

                json::string req {URL_REQUEST(forwardMessage)};

                req+=MainParser::concat_string
                (   
                    '&',
                    
                    URL_USER_INFO(chat_id, chat_id),

                    URL_USER_INFO(from_chat_id, from_chat_id),

                    URL_FIELD(message_id, std::to_string(message_id)),

                    URL_FIELD(message_thread_id, MainParser::pars_opt_as_string(message_thread_id)),

                    URL_FIELD(disable_notification, MainParser::pars_opt_as_string(disable_notification)),

                    URL_FIELD(protect, MainParser::pars_opt_as_string(protect))
                );

                return req;
            }


            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                const size_t sz = 3;
                auto map = MainParser::mapped_pointers_validation
                (
                    std::move(val),
                    std::make_pair(JS_POINTER(forwardmessage, chat_id), json::kind::string),
                    std::make_pair(JS_POINTER(forwardmessage, from_chat_id), json::kind::string),
                    std::make_pair(JS_POINTER(forwardmessage, message_id), json::kind::int64)
                );            

                if (map.size() != sz)
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
                    std::make_pair(JS_POINTER(forwardmessage, message_thread_id), json::kind::string),
                    std::make_pair(JS_POINTER(forwardmessage, disable_notification), json::kind::bool_),
                    std::make_pair(JS_POINTER(forwardmessage, protect), json::kind::bool_)
                );
            }


            template<is_fields_map T>
            void 
            fields_from_map
            (T&&  map)
            {
                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(chat_id, chat_id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(from_chat_id, from_chat_id));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(message_id, message_id));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(message_thread_id, message_thread_id));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(disable_notification, disable_notification));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(protect, protect));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return forwardMessage::fields_to_value
                (
                    forward_like<Self>(self.chat_id),
                    forward_like<Self>(self.from_chat_id),
                    self.message_id,
                    self.message_thread_id,
                    self.disable_notification,
                    self.protect
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                json::string chat_id,
                json::string from_chat_id,
                int message_id,
                optint message_thread_id,
                optbool disable_notification,
                optbool protect
            )
            {
                json::object ob  {MainParser::get_storage_ptr()};
                json::object ob_1(MainParser::get_storage_ptr());

                ob_1 =  MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(chat_id, std::move(chat_id)),
                        PAIR(from_chat_id, std::move(from_chat_id)),
                        PAIR(message_id, message_id)
                    );

                json::object ob_2{MainParser::get_storage_ptr()};
                ob_2 = MainParser::parse_OptPairs_as_obj
                      (
                        MAKE_OP(message_thread_id, message_thread_id),
                        MAKE_OP(disable_notification, disable_notification),
                        MAKE_OP(protect, protect)
                      );

                
                Pars::MainParser::container_move(std::move(ob_2), ob_1);

                ob["forwardmessage"] = { std::move(ob_1) };
                return ob;
            }

        };
    }
}