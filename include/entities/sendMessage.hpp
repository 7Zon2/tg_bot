#pragma once
#include "TelegramEntities.hpp"
#include "tg_message.hpp"

namespace Pars
{
    namespace TG
    {
        struct SendMessage : public message
        {
            static inline const json::string entity_name{"sendmessage"};
            static constexpr  size_t req_fields = 2; //chat_id, text
            static constexpr  size_t opt_fields = 17;

            public:

            size_t chat_id = {};

            public:

            SendMessage(){}

            SendMessage
            (
                const size_t chat_id,
                json::string text
            )
            :
                chat_id(chat_id),
                text(std::move(text))
            {

            }

            template<is_all_json_entities T>
            SendMessage(T&& val)
            {
                create(std::forward<T>(val));
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {

                auto message_map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(sendmessage, text), json::kind::string),
                    std::make_pair(JS_POINTER(sendmessage, chat_id), json::kind::int64),
                );

                if (message_map.size() != req_fields)
                {
                    return std::nullopt;
                }

                return message_map; 
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            optional_fields(T&& val)
            {
                
            }


            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
                MainParser::field_from_map
               <json::kind::uint64>(std::forward<T>(map), MAKE_PAIR(chat_id, chat_id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(text, text));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return SendMessage::fields_to_value
                (
                    self.chat_id,
                    Utils::forward_like<Self>(self.text)
                );
            }


            [[nodiscard]]
            static 
            json::value
            fields_to_value
            (
                size_t chat_id,
                json::string text     
            )
            {

                json::object req_ob(MainParser::get_storage_ptr());
                req_ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(chat_id, chat_id),
                        PAIR(text, text)
                    );

                return req_ob;
            }

        } 
    }

}//namespace TG