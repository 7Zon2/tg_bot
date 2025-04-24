#pragma once
#include "TelegramEntities.hpp"
#include "tg_message.hpp"
#include <type_traits>

namespace Pars
{
    namespace TG
    {
        struct  SendMessage : public message
        {
            using message::message;
            using message::operator =;

            static inline const json::string entity_name{"sendmessage"};
            static constexpr  size_t req_fields = 2; //chat_id, text
            static constexpr  size_t opt_fields = 11; //this struct is not full

            [[nodiscard]]
            json::string
            get_entity_name() noexcept override
            {
                return entity_name;
            }

            public:

            size_t chat_id = {};

            public:

            SendMessage(const message& mes):
            message(mes)
            {
              chat_id = mes.chat.id;
            }


            SendMessage(message&& mes)
            noexcept:
            message(std::move(mes))
            {
              chat_id = mes.chat.id;
            }


            SendMessage& operator = 
            (const message& mes)
            {
              SendMessage temp{mes};
              *this = std::move(temp);
              return *this;
            }


            SendMessage& operator =
            (message&& mes) noexcept 
            {
              SendMessage temp{std::move(mes)};
              *this = std::move(temp);
              return *this;
            }


            SendMessage()
            noexcept{}


            SendMessage
            (
                const size_t chat_id,
                json::string text_
            )
            noexcept:
            chat_id(chat_id)
            {
                text = std::move(text_);
            }

            template<is_all_json_entities T>
            SendMessage(T&& val)
            {
                create(std::forward<T>(val));
            }

            public:

            template<typename Self>
            [[nodiscard]]
            json::string
            fields_to_url(this Self&& self) 
            noexcept (std::is_rvalue_reference_v<Self>)
            {
                json::string id{FIELD_EQUAL(chat_id)};
                id  += std::to_string(self.chat_id);

                json::string txt{FIELD_EQUAL(text)};
                txt += Utils::forward_like<Self>(self.text).value();

                json::string req{URL_REQUEST(sendMessage)};
                URL_BIND(req, id);
                URL_BIND(req, txt);
                req.pop_back();
                return req;
            } 


            template<as_json_value T>
            [[nodiscard]]
            static opt_fields_map
            requested_fields(T&& val)
            {

                auto message_map = MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(sendmessage, text), json::kind::string),
                    std::make_pair(JS_POINTER(sendmessage, chat_id), json::kind::int64)
                );

                if (message_map.size() != req_fields)
                {
                    return std::nullopt;
                }

                return message_map; 
            }


            template<as_json_value T>
            [[nodiscard]]
            static opt_fields_map
            optional_fields(T&& val)
            {
                return {};
            }


            template<is_fields_map T>
            void
            fields_from_map
            (T && map)
            {
                MainParser::field_from_map
               <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(chat_id, chat_id));

                MainParser::field_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(text, text));
            }


            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            noexcept (std::is_rvalue_reference_v<Self>)
            {
                return SendMessage::fields_to_value
                (
                    self.chat_id,
                    Utils::forward_like<Self>(self.text)
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                size_t chat_id,
                optstr text
            )
            {

                json::object req_ob(MainParser::get_storage_ptr());
                req_ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(chat_id, chat_id)
                    );

                json::object opt_ob(MainParser::get_storage_ptr());
                opt_ob = MainParser::parse_OptPairs_as_obj
                    (
                        MAKE_OP(text,std::move(text))
                    );

                Pars::MainParser::container_move(std::move(opt_ob), req_ob);
                return req_ob;
            }

        };

    } //namespace TG

}//namespace Pars
