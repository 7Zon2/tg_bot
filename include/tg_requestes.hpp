#pragma once
#include "json_head.hpp"


namespace Pars
{
    namespace TG
    {   
        struct TelegramRequestes : MainParser
        {

            public:

            [[nodiscard]]
            static json::value
            Message
            (
                uint64_t message_id,
                uint64_t date,
                TG::Chat chat, 
                optint  message_thread_id = {},
                std::optional<User> from = {},
                std::optional<Chat> sender_chat = {},
                optint  sender_boost_count = {},
                std::optional<User> sender_business_bot = {},
                optstr  business_connection_id = {},
                std::optional<std::shared_ptr<TG::MessageOrigin>> forward_origin = {},
                optbool is_topic_message = {},
                optbool is_automatic_forward = {},
                std::optional<std::shared_ptr<message>> reply_to_message = {},
                std::optional<TG::User> via_bot = {},
                optuint edit_date = {},
                optbool has_protected_content = {},
                optbool is_from_offline = {},
                optstr media_group_id = {},
                optstr author_signature = {},
                optstr text = {}
            )
            {
                json::object ob(ptr_);
                ob = MainParser::parse_ObjPairs_as_obj
                    (
                        PAIR(message_id),
                        PAIR(date),
                        PAIR(chat)
                    );

                json::object ob2{ptr_};
                ob2 = MainParser::parse_OptPairs_as_obj
                    (

                    );  
            }
            

            [[nodiscard]]
            static json::value
            MessageOriginChannel
            (
                json::string type,
                size_t date,
                TG::Chat chat,
                size_t message_id,
                optstr author_signature = {}

            )
            {
                auto ob = MessageOriginChat(std::move(type), date, std::move(chat), std::move(author_signature)).as_object();
                ob.emplace("message_id", message_id);

                json::object res(ptr_);
                res["messageoriginchannel"] = std::move(ob);
                return res;
            }


            [[nodiscard]]
            static json::value
            get_webhook_request
            (
                json::string url,
                bool has_custom_certificate,
                uint64_t pending_update_count,
                optstr ip_address                             = {},
                optint  last_error_date                        = {},
                optstr last_error_message                     = {},
                optint last_synchronization_error_date         = {},
                optint max_connections                         = {},
                std::optional<std::vector<json::string>> allowed_updates = {}  
            )
            {
                json::object ob (ptr_);
                json::object ob1(ptr_);
                json::object ob2(ptr_);


                ob1 = parse_ObjPairs_as_obj
                    (
                        PAIR(std::move(url)),
                        PAIR(has_custom_certificate),
                        PAIR(pending_update_count)
                    );


                ob2 = parse_OptPairs_as_obj
                    (
                        MAKE_OP(std::move(ip_address)),
                        MAKE_OP(last_error_date),
                        MAKE_OP(std::move(last_error_message)),
                        MAKE_OP(last_synchronization_error_date),
                        MAKE_OP(max_connections)
                    );


                json::array arr;
                if (allowed_updates.has_value())
                {
                    auto& vec = allowed_updates.value();
                    arr = parse_all_json_as_array(vec.begin(), vec.end());
                }

                Pars::MainParser::container_move(std::move(ob2), ob1);
                ob2.clear();

                ob2["allowed_updates"] = std::move(arr);

                Pars::obj_move(std::move(ob2), ob1);

                ob ["getwebhookrequest"] = { std::move(ob1) };
                return ob;
            }


        };
    };//namespace TG

};//namespace Pars