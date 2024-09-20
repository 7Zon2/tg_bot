#pragma once
#include "TelegramEntities.hpp"
#include "concept_entities.hpp"


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

            static inline size_t req_fields = 3;
            static inline size_t opt_fields = 3;

            public:

            forwardMessage(){}

            forwardMessage
            (
                json::string_view chat_id,
                json::string_view from_chat_id,
                int message_id,
                optint message_thread_id,
                optbool disable_notification,
                optbool protect
            )
            :
             chat_id(chat_id),
             from_chat_id(from_chat_id),
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

                std::string req {URL_REQUEST(forwardMessage)};

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



            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                const size_t sz = 3;
                auto map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(forwardmessage, chat_id), json::kind::string),
                    std::make_pair(JS_POINTER(forwardmessage, from_chat_id), json::kind::string),
                    std::make_pair(JS_POINTER(forwardmessage, message_id), json::kind::int64)
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
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(chat_id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(from_chat_id));

                MainParser::field_from_map
                <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(message_id));

                MainParser::field_from_map
                <json::kind::int>(std::forward<T>(map), MAKE_PAIR(message_thread_id));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(disable_notification));

                MainParser::field_from_map
                <json::kind::bool_>(std::forward<T>(map), MAKE_PAIR(protect));
            }


            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::forwardMessage
                (
                    chat_id,
                    from_chat_id,
                    message_id,
                    message_thread_id,
                    disable_notification,
                    protect
                );
            }

        };
    }
}